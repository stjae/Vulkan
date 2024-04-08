#include "window.h"

Window::Window(int width, int height, const char* wName)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window_ = glfwCreateWindow(width, height, wName, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window_, FramebufferResizeCallback);
}

double Window::GetMousePosX()
{
    double mouseX = 0;
    double mouseY = 0;
    glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);
    return mouseX;
}

double Window::GetMousePosY()
{
    double mouseX = 0;
    double mouseY = 0;
    glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);
    return mouseY;
}

Window::~Window()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}
