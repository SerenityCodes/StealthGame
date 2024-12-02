#include "PipelineWrapper.h"

#include <stdexcept>

#include "Engine/Engine.h"
#include <glslang/Include/glslang_c_interface.h>

namespace engine::vulkan {


PipelineWrapper::PipelineWrapper(SwapChain* swap_chain, DeviceWrapper* device) : m_device_(device), m_swap_chain_(swap_chain),
    m_pipeline_layout_(nullptr), m_render_pass_(create_render_pass()), m_pipeline_(nullptr) {
    DynArray<char> vertex_shader_source = StealthEngine::read_file("Engine/Shaders/triangle.vert.spv");
    DynArray<char> fragment_shader_source = StealthEngine::read_file("Engine/Shaders/triangle.frag.spv");
    m_vertex_shader_ = create_shader_module(*device, vertex_shader_source);
    m_fragment_shader_ = create_shader_module(*device, fragment_shader_source);

    VkPipelineShaderStageCreateInfo vertex_shader_create_info{};
    vertex_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_create_info.module = m_vertex_shader_;
    vertex_shader_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_create_info{};
    fragment_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_create_info.module = m_fragment_shader_;
    fragment_shader_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = {vertex_shader_create_info, fragment_shader_create_info};

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = 2;
    dynamic_state_create_info.pDynamicStates = DYNAMIC_STATES;

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
    vertex_input_state_create_info.pVertexBindingDescriptions = nullptr;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_state_create_info.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swap_chain_->get_swap_chain_extent().width);
    viewport.height = static_cast<float>(m_swap_chain_->get_swap_chain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swap_chain_->get_swap_chain_extent();

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.lineWidth = 1.0f;
    rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
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

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(*m_device_, &pipeline_layout_create_info, nullptr, &m_pipeline_layout_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = stages;

    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisampling;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    pipeline_create_info.pDepthStencilState = nullptr;
    pipeline_create_info.layout = m_pipeline_layout_;
    pipeline_create_info.renderPass = m_render_pass_;
    pipeline_create_info.subpass = 0;
    
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(*m_device_, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &m_pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline!");
    }
}

PipelineWrapper::~PipelineWrapper() {
    for (const auto& frame_buffer : m_frame_buffers_) {
        vkDestroyFramebuffer(*m_device_, frame_buffer, nullptr);
    }
    vkDestroyShaderModule(*m_device_, m_vertex_shader_, nullptr);
    vkDestroyShaderModule(*m_device_, m_fragment_shader_, nullptr);
    vkDestroyRenderPass(*m_device_, m_render_pass_, nullptr);
    vkDestroyPipeline(*m_device_, m_pipeline_, nullptr);
    vkDestroyPipelineLayout(*m_device_, m_pipeline_layout_, nullptr);
}

VkRenderPass PipelineWrapper::create_render_pass() const {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = m_swap_chain_->get_swap_chain_format();
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;

    VkRenderPass render_pass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(*m_device_, &render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
    return render_pass;
}

void PipelineWrapper::create_frame_buffers() {
    m_frame_buffers_.resize(m_swap_chain_->get_image_views_count());
    for (uint32_t i = 0; i < m_frame_buffers_.get_size(); i++) {
        VkImageView attachments[] = {m_swap_chain_->get_image_views()[i]};
        
        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = m_render_pass_;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = attachments;
        framebuffer_create_info.width = m_swap_chain_->get_swap_chain_extent().width;
        framebuffer_create_info.height = m_swap_chain_->get_swap_chain_extent().height;
        framebuffer_create_info.layers = 1;

        if (vkCreateFramebuffer(*m_device_, &framebuffer_create_info, nullptr, &m_frame_buffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
    
}

VkPipelineLayout PipelineWrapper::get_pipeline_layout() const {
    return m_pipeline_layout_;
}

VkPipeline PipelineWrapper::get_pipeline() const {
    return m_pipeline_;
}

VkShaderModule PipelineWrapper::get_vertex_shader() const {
    return m_vertex_shader_;
}

VkShaderModule PipelineWrapper::get_fragment_shader() const {
    return m_fragment_shader_;
}

VkRenderPass PipelineWrapper::get_render_pass() const {
    return m_render_pass_;
}

VkFramebuffer PipelineWrapper::get_frame_buffer(int index) const {
    return m_frame_buffers_[index];
}

VkShaderModule PipelineWrapper::create_shader_module(const VkDevice device, const DynArray<char>& code) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.get_size();
    create_info.pCode = reinterpret_cast<uint32_t*>(code.data());

    VkShaderModule shader_module = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
    return shader_module;
}

}