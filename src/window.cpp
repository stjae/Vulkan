#include "window.h"

Window::Window(int width, int height, const char* wName)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window_ = glfwCreateWindow(width, height, wName, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window_, FramebufferResizeCallback);
}

Window::~Window()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}
