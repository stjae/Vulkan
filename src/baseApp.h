#ifndef _BASEAPP_H_
#define _BASEAPP_H_

#include "common.h"

class HelloTriangleApplication
{
public:
    HelloTriangleApplication(const int width, const int height, const char* wName) : m_window(width, height, wName) {}
    void run();

private:
    void InitVulkan();
    void MainLoop();

    Window m_window;
};

#endif