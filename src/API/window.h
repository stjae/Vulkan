#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "../common.h"

class Window
{
    const int width_;
    const int height_;

public:
    Window(const int width, const int height, const char* wName);
    ~Window();

    static GLFWwindow** GetWindow() { static GLFWwindow* window; return &window; }
};

#endif