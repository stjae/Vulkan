#include "window.h"

Window::Window(int width, int height, const char* wName) : width(width), height(height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, wName, nullptr, nullptr);
}

int Window::ShouldClose()
{
    return glfwWindowShouldClose(window);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

GLFWwindow* Window::window;