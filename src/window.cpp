#include "window.h"
#include "keycode.h"

Window::Window(int width, int height, const char* title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    s_glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(s_glfwWindow, FramebufferResizeCallback);
}

double Window::GetMousePosX()
{
    double mouseX, mouseY;
    glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);
    return mouseX;
}

double Window::GetMousePosY()
{
    double mouseX, mouseY;
    glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);
    return mouseY;
}

float Window::GetMousePosNormalizedX()
{
    double mouseX, mouseY;
    glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);

    int width, height;
    glfwGetWindowSize(Window::GetWindow(), &width, &height);
    mouseX /= (float)width;
    mouseX -= 0.5f;

    return (float)mouseX;
}

float Window::GetMousePosNormalizedY()
{
    double mouseX, mouseY;
    glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);

    int width, height;
    glfwGetWindowSize(Window::GetWindow(), &width, &height);
    mouseY /= (float)height;
    mouseY -= 0.5f;

    return (float)mouseY;
}

bool Window::IsKeyDown(Keycode keycode)
{
    return glfwGetKey(Window::GetWindow(), keycode);
}

Window::~Window()
{
    glfwDestroyWindow(s_glfwWindow);
    glfwTerminate();
}
