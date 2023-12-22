#ifndef WINDOW_H
#define WINDOW_H

#include "../common.h"

class Window
{
    const int width_;
    const int height_;

public:
    Window(const int width, const int height, const char* wName);
    ~Window();

    static GLFWwindow** GetWindow()
    {
        static GLFWwindow* window;
        return &window;
    }
};

#endif