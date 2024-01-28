#include "window.h"

Window::Window(int width, int height, const char* wName)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    *GetWindow() = glfwCreateWindow(width, height, wName, nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(*GetWindow());
    glfwTerminate();
}