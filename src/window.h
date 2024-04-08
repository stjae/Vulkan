#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

class Window
{
    inline static GLFWwindow* window_;

public:
    inline static bool resized = false;

    Window(int width, int height, const char* wName);
    static GLFWwindow* GetWindow() { return window_; }
    static double GetMousePosX();
    static double GetMousePosY();
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) { resized = true; }
    ~Window();
};

#endif