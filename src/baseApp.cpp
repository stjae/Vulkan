#include "baseApp.h"

void HelloTriangleApplication::run()
{
    InitVulkan();
    MainLoop();
}

void HelloTriangleApplication::InitVulkan()
{
    
}

void HelloTriangleApplication::MainLoop()
{
    while (!m_window.ShouldClose()) {
        glfwPollEvents();
    }
}
