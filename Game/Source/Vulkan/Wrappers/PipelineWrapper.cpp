#include "PipelineWrapper.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <iostream>

#include "Engine.h"
#include "FileIO/FileIO.h"
#include "Vulkan/VulkanModel.h"
#include <spirv_reflect.h>

namespace engine::vulkan {

TBuiltInResource get_resources() {
    TBuiltInResource resources;
    resources.maxLights = 32;
    resources.maxClipPlanes = 6;
    resources.maxTextureUnits = 32;
    resources.maxTextureCoords = 32;
    resources.maxVertexAttribs = 64;
    resources.maxVertexUniformComponents = 4096;
    resources.maxVaryingFloats = 64;
    resources.maxVertexTextureImageUnits = 32;
    resources.maxCombinedTextureImageUnits = 80;
    resources.maxTextureImageUnits = 32;
    resources.maxFragmentUniformComponents = 4096;
    resources.maxDrawBuffers = 32;
    resources.maxVertexUniformVectors = 128;
    resources.maxVaryingVectors = 8;
    resources.maxFragmentUniformVectors = 16;
    resources.maxVertexOutputVectors = 16;
    resources.maxFragmentInputVectors = 15;
    resources.minProgramTexelOffset = -8;
    resources.maxProgramTexelOffset = 7;
    resources.maxClipDistances = 8;
    resources.maxComputeWorkGroupCountX = 65535;
    resources.maxComputeWorkGroupCountY = 65535;
    resources.maxComputeWorkGroupCountZ = 65535;
    resources.maxComputeWorkGroupSizeX = 1024;
    resources.maxComputeWorkGroupSizeY = 1024;
    resources.maxComputeWorkGroupSizeZ = 64;
    resources.maxComputeUniformComponents = 1024;
    resources.maxComputeTextureImageUnits = 16;
    resources.maxComputeImageUniforms = 8;
    resources.maxComputeAtomicCounters = 8;
    resources.maxComputeAtomicCounterBuffers = 1;
    resources.maxVaryingComponents = 60;
    resources.maxVertexOutputComponents = 64;
    resources.maxGeometryInputComponents = 64;
    resources.maxGeometryOutputComponents = 128;
    resources.maxFragmentInputComponents = 128;
    resources.maxImageUnits = 8;
    resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    resources.maxCombinedShaderOutputResources = 8;
    resources.maxImageSamples = 0;
    resources.maxVertexImageUniforms = 0;
    resources.maxTessControlImageUniforms = 0;
    resources.maxTessEvaluationImageUniforms = 0;
    resources.maxGeometryImageUniforms = 0;
    resources.maxFragmentImageUniforms = 8;
    resources.maxCombinedImageUniforms = 8;
    resources.maxGeometryTextureImageUnits = 16;
    resources.maxGeometryOutputVertices = 256;
    resources.maxGeometryTotalOutputComponents = 1024;
    resources.maxGeometryUniformComponents = 1024;
    resources.maxGeometryVaryingComponents = 64;
    resources.maxTessControlInputComponents = 128;
    resources.maxTessControlOutputComponents = 128;
    resources.maxTessControlTextureImageUnits = 16;
    resources.maxTessControlUniformComponents = 1024;
    resources.maxTessControlTotalOutputComponents = 4096;
    resources.maxTessEvaluationInputComponents = 128;
    resources.maxTessEvaluationOutputComponents = 128;
    resources.maxTessEvaluationTextureImageUnits = 16;
    resources.maxTessEvaluationUniformComponents = 1024;
    resources.maxTessPatchComponents = 120;
    resources.maxPatchVertices = 32;
    resources.maxTessGenLevel = 64;
    resources.maxViewports = 16;
    resources.maxVertexAtomicCounters = 0;
    resources.maxTessControlAtomicCounters = 0;
    resources.maxTessEvaluationAtomicCounters = 0;
    resources.maxGeometryAtomicCounters = 0;
    resources.maxFragmentAtomicCounters = 8;
    resources.maxCombinedAtomicCounters = 8;
    resources.maxAtomicCounterBindings = 1;
    resources.maxVertexAtomicCounterBuffers = 0;
    resources.maxTessControlAtomicCounterBuffers = 0;
    resources.maxTessEvaluationAtomicCounterBuffers = 0;
    resources.maxGeometryAtomicCounterBuffers = 0;
    resources.maxFragmentAtomicCounterBuffers = 1;
    resources.maxCombinedAtomicCounterBuffers = 1;
    resources.maxAtomicCounterBufferSize = 16384;
    resources.maxTransformFeedbackBuffers = 4;
    resources.maxTransformFeedbackInterleavedComponents = 64;
    resources.maxCullDistances = 8;
    resources.maxCombinedClipAndCullDistances = 8;
    resources.maxSamples = 4;

    resources.limits.nonInductiveForLoops = true;
    resources.limits.whileLoops = true;
    resources.limits.doWhileLoops = true;
    resources.limits.generalUniformIndexing = true;
    resources.limits.generalAttributeMatrixVectorIndexing = true;
    resources.limits.generalVaryingIndexing = true;
    resources.limits.generalSamplerIndexing = true;
    resources.limits.generalVariableIndexing = true;
    resources.limits.generalConstantMatrixVectorIndexing = true;
    return resources;
}

arena_vector<Shader> parse_and_compile_shaders(Arena& temp_arena, io::Folder& path, arena_vector<glslang::TShader*>* shaders_to_remove) {
    arena_vector<glslang::TShader*> shaders = MAKE_ARENA_VECTOR(&temp_arena, glslang::TShader*);
    arena_vector<Shader> ret_shaders = MAKE_ARENA_VECTOR(&temp_arena, Shader);

    EShMessages messages = EShMsgDefault;

    TBuiltInResource resources = get_resources();
    glslang::InitializeProcess();
    glslang::TProgram program;
    for (int i = 0; i < static_cast<int>(path.get_amount_of_files()); i++) {
        io::RawFile shader_file = path.read_file(i);
        arena_string extension = shader_file.get_file_extension();
        arena_string& full_path = shader_file.get_file_path();
        ENGINE_LOG_INFO("Parsing shader %s", full_path);
        ENGINE_LOG_INFO("%s", extension);
        bool is_vertex = extension == "vert";
        bool is_fragment = extension == "frag";
        if (is_vertex || is_fragment) {
            arena_string shader_contents = shader_file.read_contents();
            EShLanguage stage = is_vertex ? EShLangVertex : EShLangFragment;
            void* shader_memory = temp_arena.push(sizeof(glslang::TShader), alignof(glslang::TShader));
            glslang::TShader* shader = new (shader_memory) glslang::TShader{stage};

            const char* raw_data = shader_contents.c_str();
            shader->setStrings(&raw_data, 1);
            shader->setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 450);
            shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
            shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
            if (!shader->parse(&resources, 450, false, messages)) {
                ENGINE_LOG_ERROR("Shader compilation failed for {}. Shader log: \n{}\n", full_path, shader->getInfoLog());
                continue;
            }
            Shader& ret_shader = ret_shaders.emplace_back(GLOBAL_VERTEX_SHADER, VK_SHADER_STAGE_VERTEX_BIT, std::vector<unsigned int>{});
            load_shader(ret_shader, full_path, extension);
            shaders.push_back(shader);
            program.addShader(shader);
        }
    }
    ENGINE_ASSERT(program.link(messages), "Shader program linking failed. Program log: {}", program.getInfoLog())
    for (int i = 0; i < static_cast<int>(ret_shaders.size()); i++) {
        glslang::TShader* shader = shaders[i];
        glslang::TIntermediate* intermediate = shader->getIntermediate();
        Shader& ret_shader = ret_shaders[i];
        if (intermediate) {
            GlslangToSpv(*intermediate, ret_shader.spirv_code);
        }
    }
    glslang::FinalizeProcess();
    *shaders_to_remove = arena_vector<glslang::TShader*>{shaders.begin(), shaders.end(), &temp_arena};
    return ret_shaders;
}

arena_vector<VkDescriptorSetLayout> spirv_reflect_descriptor_pool(Arena& temp_arena, VkDevice device, const arena_vector<Shader>& shaders) {
    arena_vector<VkDescriptorSetLayout> all_descriptor_sets = MAKE_ARENA_VECTOR(&temp_arena, VkDescriptorSetLayout);
    for (const Shader& shader : shaders) {
        SpvReflectShaderModule module;
        SpvReflectResult result = spvReflectCreateShaderModule(shader.spirv_code.size() * sizeof(u32), shader.spirv_code.data(), &module);
        ENGINE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Reflection failed")

        u32 count = 0;
        result = spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
        ENGINE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Descriptor set enumeration failed")

        arena_vector<SpvReflectDescriptorSet*> reflect_sets = MAKE_ARENA_VECTOR(&temp_arena, SpvReflectDescriptorSet*);
        reflect_sets.resize(count, nullptr);
        result = spvReflectEnumerateDescriptorSets(&module, &count, reflect_sets.data());
        ENGINE_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Descriptor set enumeration failed")

        arena_vector<PipelineWrapper::DescriptorSetLayoutData> descriptor_set_layouts = MAKE_ARENA_VECTOR(&temp_arena, PipelineWrapper::DescriptorSetLayoutData);
        descriptor_set_layouts.resize(count, PipelineWrapper::DescriptorSetLayoutData{.set_number = 0, .create_info = {}, .bindings = {{}, STLArenaAllocator<VkDescriptorSetLayoutBinding>{&temp_arena}}});
        for (u64 set_num = 0; set_num < count; set_num++) {
            SpvReflectDescriptorSet* reflect_set = reflect_sets[set_num];
            PipelineWrapper::DescriptorSetLayoutData& layout_data = descriptor_set_layouts[set_num];
            layout_data.bindings.resize(reflect_set->binding_count);
            for (u32 binding = 0; binding < reflect_set->binding_count; binding++) {
                SpvReflectDescriptorBinding& reflect_binding = *reflect_set->bindings[binding];
                VkDescriptorSetLayoutBinding& layout_binding = layout_data.bindings[binding];
                layout_binding.binding = reflect_binding.binding;
                layout_binding.descriptorType = static_cast<VkDescriptorType>(reflect_binding.descriptor_type);
                layout_binding.descriptorCount = 1;
                for (u32 dim = 0; dim < reflect_binding.array.dims_count; dim++) {
                    layout_binding.descriptorCount *= reflect_binding.array.dims[dim];
                }
                layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);
            }
            layout_data.set_number = reflect_set->set;
            layout_data.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout_data.create_info.bindingCount = reflect_set->binding_count;
            layout_data.create_info.pBindings = layout_data.bindings.data();
        }

        // Merge duplicates
        arena_vector<VkDescriptorSetLayout> descriptor_sets = MAKE_ARENA_VECTOR(&temp_arena, VkDescriptorSetLayout);
        descriptor_sets.resize(count, VkDescriptorSetLayout{});
        for (u64 set_num = 0; set_num < count; set_num++) {
            VULKAN_ASSERT(vkCreateDescriptorSetLayout(device, &descriptor_set_layouts[set_num].create_info, nullptr, &descriptor_sets[set_num]), "Failed to create descriptor set {}", set_num)
        }
        for (auto it = descriptor_sets.begin(); it != descriptor_sets.end(); ++it) {
            all_descriptor_sets.push_back(*it);
        }
    }
    return all_descriptor_sets;
} 

PipelineWrapper::PipelineWrapper(Arena& temp_arena, VulkanRenderer& renderer) : m_device_(renderer.vulkan_device()), m_shaders_(MAKE_ARENA_VECTOR(&temp_arena, VkShaderModule)) {
    io::Folder shader_folder{&temp_arena, "Shaders/"};
    arena_vector<glslang::TShader*> shaders_to_remove = MAKE_ARENA_VECTOR(&temp_arena, glslang::TShader*);
    arena_vector<Shader> shaders = parse_and_compile_shaders(temp_arena, shader_folder, &shaders_to_remove);
    for (glslang::TShader* shader : shaders_to_remove) {
        shader->~TShader();
    }
    m_shaders_.resize(shaders.size(), VkShaderModule{});
    for (u64 i = 0; i < shaders.size(); i++) {
        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        std::vector<u32>& compiled_code = shaders[i].spirv_code;
        shader_module_create_info.pCode = compiled_code.data();
        shader_module_create_info.codeSize = compiled_code.size() * sizeof(u32);
        VULKAN_ASSERT(vkCreateShaderModule(m_device_, &shader_module_create_info, nullptr, &m_shaders_[i]), "Failed to create shader module {}", i)
    }

    arena_vector<VkPipelineShaderStageCreateInfo> stages = MAKE_ARENA_VECTOR(&temp_arena, VkPipelineShaderStageCreateInfo);
    stages.reserve(shaders.size());
    for (u64 i = 0; i < shaders.size(); i++) {
        VkPipelineShaderStageCreateInfo stage{};
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.stage = shaders[i].shader_stage;
        stage.module = m_shaders_[i];
        stage.pName = "main";
        stages.push_back(stage);
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

    arena_vector<VkDescriptorSetLayout> sets = spirv_reflect_descriptor_pool(temp_arena, m_device_, shaders);
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

}