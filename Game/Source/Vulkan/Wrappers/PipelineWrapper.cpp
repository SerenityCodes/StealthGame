#include "PipelineWrapper.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <iostream>

#include "Engine.h"
#include "FileIO/FileIO.h"
#include "Vulkan/VulkanModel.h"
#include <spirv_reflect.h>

namespace engine::vulkan {

DynArray<Shader> parse_and_compile_shaders(Arena& temp_arena, io::Folder& path) {
    DynArray<glslang::TShader*> shaders{temp_arena};
    DynArray<Shader> ret_shaders{temp_arena};

    EShMessages messages = EShMsgDefault;
    
    glslang::InitializeProcess();
    glslang::TProgram program;
    for (int i = 0; i < static_cast<int>(path.get_amount_of_files()); i++) {
        io::RawFile shader_file = path.read_file(i);
        String extension = shader_file.get_file_extension();
        String& full_path = shader_file.get_file_path();
        bool is_vertex = extension == "vert";
        bool is_fragment = extension == "frag";
        if (is_vertex || is_fragment) {
            DynArray<byte> raw_code = shader_file.read_raw_bytes();
            EShLanguage stage = is_vertex ? EShLangVertex : EShLangFragment;
            glslang::TShader* shader = new (static_cast<glslang::TShader*>(temp_arena.push(sizeof(glslang::TShader)))) glslang::TShader(stage);
            
            const char* shader_code = reinterpret_cast<const char*>(raw_code.data());
            shader->setStrings(&shader_code, i);
            TBuiltInResource resources{};
            if (!shader->parse(&resources, 450, false, messages)) {
                ENGINE_LOG_ERROR("Shader compilation failed for {}. Shader log: {}", full_path.c_str(temp_arena), shader->getInfoLog());
                continue;
            }
            ret_shaders.push_back(load_shader(temp_arena, full_path, extension));
            shaders.push_back(shader);
            program.addShader(shader);
        }
    }
    ENGINE_ASSERT(program.link(messages), "Shader program linking failed. Program log: {}", program.getInfoLog())

    for (int i = 0; i < static_cast<int>(ret_shaders.size()); i++) {
        glslang::TShader* shader = shaders[i];
        glslang::TIntermediate* intermediate = shader->getIntermediate();
        if (intermediate) {
            Shader& shader_obj = ret_shaders[i];
            GlslangToSpv(*intermediate, shader_obj.spirv_code);
        }
    }
    glslang::FinalizeProcess();
    return ret_shaders;
}

DynArray<VkDescriptorSetLayout> spirv_reflect_descriptor_pool(Arena& temp_arena, VkDevice device, const DynArray<Shader>& shaders) {
    DynArray<VkDescriptorSetLayout> all_descriptor_sets(temp_arena);
    for (const Shader& shader : shaders) {
        SpvReflectShaderModule module;
        SpvReflectResult result = spvReflectCreateShaderModule(shader.spirv_code.size(), shader.spirv_code.data(), &module);
        ENGINE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Reflection failed")

        u32 count = 0;
        result = spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
        ENGINE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Descriptor set enumeration failed")

        DynArray<SpvReflectDescriptorSet*> reflect_sets(temp_arena, count);
        result = spvReflectEnumerateDescriptorSets(&module, &count, reflect_sets.data());
        ENGINE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Descriptor set enumeration failed")

        DynArray<PipelineWrapper::DescriptorSetLayoutData> descriptor_set_layouts(temp_arena, count);
        for (u64 set_num = 0; set_num < count; set_num++) {
            SpvReflectDescriptorSet& reflect_set = *reflect_sets[set_num];
            PipelineWrapper::DescriptorSetLayoutData& layout_data = descriptor_set_layouts[set_num];
            layout_data.bindings.resize(reflect_set.binding_count);
            for (u32 binding = 0; binding < reflect_set.binding_count; binding++) {
                SpvReflectDescriptorBinding& reflect_binding = *reflect_set.bindings[binding];
                VkDescriptorSetLayoutBinding& layout_binding = layout_data.bindings[binding];
                layout_binding.binding = reflect_binding.binding;
                layout_binding.descriptorType = static_cast<VkDescriptorType>(reflect_binding.descriptor_type);
                layout_binding.descriptorCount = 1;
                for (u32 dim = 0; dim < reflect_binding.array.dims_count; dim++) {
                    layout_binding.descriptorCount *= reflect_binding.array.dims[dim];
                }
                layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);
            }
            layout_data.set_number = reflect_set.set;
            layout_data.create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            layout_data.create_info.bindingCount = reflect_set.binding_count;
            layout_data.create_info.pBindings = layout_data.bindings.data();
        }

        // Merge duplicates
        DynArray<VkDescriptorSetLayout> descriptor_sets(temp_arena, count);
        for (u64 set_num = 0; set_num < count; set_num++) {
            VULKAN_ASSERT(vkCreateDescriptorSetLayout(device, &descriptor_set_layouts[set_num].create_info, nullptr, &descriptor_sets[set_num]), "Failed to create descriptor set {}", set_num)
        }
        all_descriptor_sets.add_from_container(descriptor_sets.begin(), descriptor_sets.end());
    }
    return all_descriptor_sets;
} 

PipelineWrapper::PipelineWrapper(Arena& temp_arena, VulkanRenderer& renderer) : m_device_(renderer.vulkan_device()), m_shaders_(temp_arena) {
    io::Folder shader_folder{&temp_arena, "Shaders/"};
    DynArray<Shader> shaders = parse_and_compile_shaders(temp_arena, shader_folder);
    m_shaders_.resize(shaders.size());
    for (u64 i = 0; i < shaders.size(); i++) {
        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        std::vector<u32>& compiled_code = shaders[i].spirv_code;
        shader_module_create_info.pCode = compiled_code.data();
        shader_module_create_info.codeSize = compiled_code.size();
        VULKAN_ASSERT(vkCreateShaderModule(m_device_, &shader_module_create_info, nullptr, &m_shaders_[i]), "Failed to create shader module {}", i)
    }

    DynArray<VkPipelineShaderStageCreateInfo> stages{temp_arena};
    for (u64 i = 0; i < shaders.size(); i++) {
        stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[i].stage = shaders[i].shader_stage;
        stages[i].module = m_shaders_[i];
        stages[i].pName = "main";
    }
    
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = 2;
    dynamic_state_create_info.pDynamicStates = dynamic_states;

    VkVertexInputBindingDescription binding_description = VulkanModel::Vertex::get_binding_descriptions();
    auto vertex_attribute_descriptions = VulkanModel::Vertex::get_attribute_descriptions();
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_state_create_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions.size());
    vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(renderer.get_swap_chain_extent().width);
    viewport.height = static_cast<float>(renderer.get_swap_chain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = renderer.get_swap_chain_extent();

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.lineWidth = 1.0f;
    rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
    rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_state{};
    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.logicOpEnable = VK_FALSE;
    color_blend_state.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &color_blend_attachment;
    color_blend_state.blendConstants[0] = 0.0f; // Optional
    color_blend_state.blendConstants[1] = 0.0f; // Optional
    color_blend_state.blendConstants[2] = 0.0f; // Optional
    color_blend_state.blendConstants[3] = 0.0f; // Optional

    DynArray<VkDescriptorSetLayout> sets = spirv_reflect_descriptor_pool(temp_arena, m_device_, shaders);
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = static_cast<u32>(sets.size());
    pipeline_layout_create_info.pSetLayouts = sets.data();

    VULKAN_ASSERT(vkCreatePipelineLayout(renderer.vulkan_device(), &pipeline_layout_create_info, nullptr, &m_pipeline_layout_), "Failed to create pipeline layout!")

    VkGraphicsPipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = static_cast<u32>(stages.size());
    pipeline_create_info.pStages = stages.data();

    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisampling;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    pipeline_create_info.pDepthStencilState = nullptr;
    pipeline_create_info.layout = m_pipeline_layout_;
    pipeline_create_info.renderPass = renderer.render_pass();
    pipeline_create_info.subpass = 0;
    
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    VULKAN_ASSERT(vkCreateGraphicsPipelines(renderer.vulkan_device(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &m_pipeline_), "Failed to create pipeline!")
}

VkShaderModule PipelineWrapper::load_shader_code(ShaderEnum shader_to_load, Arena& temp_arena) const {
    const char* file_path;
    switch (shader_to_load) {
        case GLOBAL_VERTEX_SHADER:
            file_path = "Shaders/global.vert";
            break;
        case GLOBAL_FRAGMENT_SHADER:
            file_path = "Shaders/global.frag";
            break;
        default:
            file_path = "";
    }
    io::RawFile file{&temp_arena, file_path};
    DynArray<byte> spirv_code = file.read_raw_bytes();
    return create_shader_module(m_device_, spirv_code);
}

PipelineWrapper::~PipelineWrapper() {
    vkDeviceWaitIdle(m_device_);
    for (const VkShaderModule shader_module : m_shaders_) {
        vkDestroyShaderModule(m_device_, shader_module, nullptr);
    }
    vkDestroyPipelineLayout(m_device_, m_pipeline_layout_, nullptr);
    vkDestroyPipeline(m_device_, m_pipeline_, nullptr);
}

VkPipelineLayout PipelineWrapper::get_pipeline_layout() const {
    return m_pipeline_layout_;
}

VkPipeline PipelineWrapper::get_pipeline() const {
    return m_pipeline_;
}

void PipelineWrapper::bind(VkCommandBuffer command_buffer) const {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_);
}

VkShaderModule PipelineWrapper::create_shader_module(const VkDevice device, const DynArray<byte>& code) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<u32*>(code.data());

    VkShaderModule shader_module = VK_NULL_HANDLE;
    VULKAN_ASSERT(vkCreateShaderModule(device, &create_info, nullptr, &shader_module), "Failed to create shader module!")
    return shader_module;
}

}