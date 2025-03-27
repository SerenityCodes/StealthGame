#pragma once

#include "GLFW/glfw3.h"

namespace engine {

class Window {
    GLFWwindow* m_window_;
    int m_height_, m_width_;
    bool m_was_resized_ = false;
public:
    Window(int height, int width, const char* title);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;
    ~Window();

    GLFWwindow* raw_window() const;
    operator GLFWwindow*() const;

    bool should_close() const;
    static void glfw_poll_events();

    void set_resized();
    void toggle_resized();
    bool was_resized() const;

    static void frame_buffer_size_callback(GLFWwindow* window, int width, int height);
};

}
