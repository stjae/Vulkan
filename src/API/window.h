#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "../common.h"

struct Window {
    Window(const int width, const int height, const char* wName);
    ~Window();

    static GLFWwindow*& Get();

    int ShouldClose();

    const int width;
    const int height;
};

#endif