#include "Window.h"

namespace engine {

Window::Window(const int height, const int width, const char* title) : m_window_(nullptr), m_height_(height), m_width_(width) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
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

}