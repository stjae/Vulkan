#include "window.h"

Window::Window(int width, int height, const char* wName) : m_width(width), m_height(height)
{
    glfwInit();
    m_windowPtr = glfwCreateWindow(width, height, wName, nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(m_windowPtr);
    glfwTerminate();
}

int Window::ShouldClose()
{
    return glfwWindowShouldClose(m_windowPtr);
}
