#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "../common.h"

struct Window {
    Window(const int width, const int height, const char* wName);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    int ShouldClose();

    const int width;
    const int height;

    GLFWwindow* window;
};

#endif