#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "../common.h"

class Window
{
public:
    Window(const int width, const int height, const char* wName);
    ~Window();

    const int m_width;
    const int m_height;

    GLFWwindow* m_window;
};

#endif