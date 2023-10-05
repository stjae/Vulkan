#include "window.h"

Window::Window(int width, int height, const char* wName) : width(width), height(height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    Window::Get() = glfwCreateWindow(width, height, wName, nullptr, nullptr);
}

int Window::ShouldClose()
{
    return glfwWindowShouldClose(Window::Get());
}

Window::~Window()
{
    glfwDestroyWindow(Window::Get());
    glfwTerminate();
}

// getter
GLFWwindow*& Window::Get()
{
    static GLFWwindow* window;
    return window;
}