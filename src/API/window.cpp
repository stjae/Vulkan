#include "window.h"

Window::Window(int width, int height, const char* wName) : width(width), height(height)
{
    glfwInit();
    window = glfwCreateWindow(width, height, wName, nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int Window::ShouldClose()
{
    return glfwWindowShouldClose(window);
}
