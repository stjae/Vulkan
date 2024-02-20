#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

class Window
{
    inline static GLFWwindow* window_;

public:
    Window(int width, int height, const char* wName);
    ~Window();

    static GLFWwindow* GetWindow() { return window_; }
};

#endif