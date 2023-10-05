#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : m_window(width, height, wName) {}

void Application::run()
{
    InitVulkan();
    MainLoop();
}

void Application::InitVulkan()
{
    m_instance.Create();
    m_log.CreateDebugMessenger();
    m_instance.CreateSurface();

    m_device.PickPhysicalDevice();
    m_device.FindQueueFamilies();
    m_device.CreateLogicalDevice();
}

void Application::MainLoop()
{
    while (!m_window.ShouldClose()) {
        glfwPollEvents();
    }
}
