#include "Window.h"

namespace engine {

Window::Window(const int height, const int width, const char* title) : m_window_(nullptr), m_height_(height), m_width_(width) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_window_, this);
    glfwSetFramebufferSizeCallback(m_window_, frame_buffer_size_callback);
}

Window::~Window() {
    glfwDestroyWindow(m_window_);
    glfwTerminate();
}

GLFWwindow* Window::raw_window() const {
    return m_window_;
}

Window::operator GLFWwindow*() const {
    return m_window_;
}

bool Window::should_close() const {
    return glfwWindowShouldClose(m_window_);
}

void Window::glfw_poll_events() {
    glfwPollEvents();
}

void Window::set_resized() {
    m_was_resized_ = true;
}

void Window::toggle_resized() {
    m_was_resized_ = !m_was_resized_;
}

bool Window::was_resized() {
    return m_was_resized_;
}

void Window::frame_buffer_size_callback(GLFWwindow* window, int width,
                                        int height) {
    auto window_ptr = static_cast<Window*>(glfwGetWindowUserPointer(window));
    window_ptr->set_resized();
}

}