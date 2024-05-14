#include "window.h"

Window::Window(int width, int height, const char* title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    s_glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(s_glfwWindow, FramebufferResizeCallback);
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
    glfwDestroyWindow(s_glfwWindow);
    glfwTerminate();
}
