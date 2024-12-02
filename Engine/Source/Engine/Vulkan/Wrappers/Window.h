#pragma once

#include "GLFW/glfw3.h"

namespace engine {

class Window {
    GLFWwindow* m_window_;
    int m_height_, m_width_;
public:
    Window(int height, int width, const char* title);
    ~Window();

    GLFWwindow* raw_window() const;
    operator GLFWwindow*() const;

    bool should_close() const;
    void glfw_poll_events();
};

}
