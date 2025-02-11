#include "Engine.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "common.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Systems/CoreEngineSystems.h"
#include "Vulkan/VulkanRenderInfo.h"
#include "Logging/Logger.h"

void* operator new(size_t size) {
    std::cout << "Allocated bytes: " << size << "\n";
    return malloc(size);
}

void operator delete(void* p) noexcept {
    //std::cout << "Deleted 0x" << std::hex << p << "\n";
    free(p);
}

void* operator new[](size_t size) {
    std::cout << "Allocated bytes: " << size << "\n";
    return malloc(size);
}

void operator delete[](void* p) noexcept {
    free(p);
} 

namespace engine {

constexpr int default_stack_size = 2 << 25;

void check_result_func(VkResult result) {
    ENGINE_ASSERT(result == VK_SUCCESS, "ImGUI Check Result Function failed")
}

void setup_imgui(Window& window, const vulkan::InstanceWrapper& instance, const vulkan::DeviceWrapper& device, const vulkan::BasicRenderer& renderer, const vulkan::SwapChain& swap_chain, const vulkan::DescriptorPool& descriptor_pool) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window.raw_window(), true);
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = instance;
    init_info.PhysicalDevice = device.get_physical_device();
    init_info.Device = device.get_logical_device();
    init_info.QueueFamily = device.get_graphics_queue_family().graphics_family_index;
    init_info.Queue = device.get_graphics_queue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.RenderPass = renderer.get_render_pass();
    init_info.Subpass = 0;
    init_info.DescriptorPool = descriptor_pool.descriptor_pool();
    init_info.MinImageCount = swap_chain.get_image_count();
    init_info.ImageCount = swap_chain.get_image_count();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = check_result_func;
    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_SetMinImageCount(swap_chain.get_image_count());
}

StealthEngine::StealthEngine() : m_temp_arena_((Logger::Init(), default_stack_size)),
     m_permanent_arena_(default_stack_size),
    m_vulkan_wrapper_(m_temp_arena_),
    m_renderer_(m_temp_arena_, m_permanent_arena_,
    &m_vulkan_wrapper_.window(), m_vulkan_wrapper_.device(), m_vulkan_wrapper_.surface()),
    m_pipeline_(m_temp_arena_, &m_renderer_, m_vulkan_wrapper_.device())
    {
    setup_imgui(m_vulkan_wrapper_.window(), m_vulkan_wrapper_.instance(), *m_vulkan_wrapper_.device(), m_renderer_, m_renderer_.get_swap_chain(), m_vulkan_wrapper_.descriptor_pool());
}

void StealthEngine::run() {
    ENGINE_LOG_INFO("Engine starting...")
    m_world_.add<VulkanRenderInfo>();
    m_world_.set<components::WindowComponent>({m_vulkan_wrapper_.window().raw_window()});
    systems::setup_render_system(m_world_);
    bool should_continue = true;
    bool p_open = true;
    ImGui_ImplVulkan_CreateFontsTexture();
    while (!m_vulkan_wrapper_.window().should_close() && should_continue) {
        m_temp_arena_.clear();
        m_vulkan_wrapper_.window().glfw_poll_events();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&p_open);
        if (const auto cmd_buffer = m_renderer_.begin_frame(m_temp_arena_, m_permanent_arena_)) {
            m_renderer_.begin_render_pass(cmd_buffer);
            m_renderer_.bind_pipeline(m_pipeline_.get_pipeline());
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();
            ImGui_ImplVulkan_RenderDrawData(draw_data, cmd_buffer);
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
            VulkanRenderInfo* render_info = m_world_.get_mut<VulkanRenderInfo>();
            render_info->cmd_buffer = cmd_buffer;
            render_info->pipeline_layout = m_pipeline_.get_pipeline_layout();
            should_continue = m_world_.progress();
            m_renderer_.end_render_pass(cmd_buffer);
            m_renderer_.end_frame(m_temp_arena_, m_permanent_arena_);
        }
    }
    // Join Threads Here
    vkDeviceWaitIdle(*m_vulkan_wrapper_.device());
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

flecs::world& StealthEngine::get_world() {
    return m_world_;
}

vulkan::VulkanModel StealthEngine::create_model(
    const vulkan::VulkanModel::VertexIndexInfo& index_info) {
    return {m_vulkan_wrapper_.device(), m_renderer_.get_command_pool(), index_info};
}

vulkan::VulkanModel StealthEngine::load_model(const String& file_name, uint32_t import_flags) {
    return vulkan::VulkanModel::load_model(m_temp_arena_, m_permanent_arena_, m_vulkan_wrapper_.device(), m_renderer_.get_command_pool(), file_name.c_str(m_temp_arena_), import_flags);
}

vulkan::VulkanModel StealthEngine::load_model(const char* file_name, uint32_t import_flags) {
    return vulkan::VulkanModel::load_model(m_temp_arena_, m_permanent_arena_, m_vulkan_wrapper_.device(), m_renderer_.get_command_pool(), file_name, import_flags);
}

float StealthEngine::get_aspect_ratio() const {
    return m_renderer_.get_aspect_ratio();
}

ArrayRef<byte> StealthEngine::read_temporary_file(Arena& temp_arena, const String& file_name) {
    return read_temporary_file(temp_arena, file_name.c_str(temp_arena));
}

ArrayRef<byte> StealthEngine::read_temporary_file(Arena& temp_arena, const char* file_name) {
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    ENGINE_ASSERT(file.is_open(), "Failed to open file")
    file.seekg(0, std::ios::end);
    const std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    const size_t bytes_needed = sizeof(byte) * file_size;
    byte* ptr = static_cast<byte*>(temp_arena.push(bytes_needed));
    ArrayRef buffer{ptr, static_cast<size_t>(file_size)};
    file.read(reinterpret_cast<char*>(ptr), file_size);
    file.close();
    return buffer;
}


}