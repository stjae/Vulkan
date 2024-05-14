#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

class Window
{
    inline static GLFWwindow* s_glfwWindow;

public:
    inline static bool s_resized = false;

    Window(int width, int height, const char* title);
    static GLFWwindow* GetWindow() { return s_glfwWindow; }
    static double GetMousePosX();
    static double GetMousePosY();
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) { s_resized = true; }
    ~Window();
};

#endif