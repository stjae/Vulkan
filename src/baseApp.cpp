#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName, bool debugMode)
    : m_window(width, height, wName),
      m_instance(debugMode) {}

void Application::run()
{
    InitVulkan();
    MainLoop();
}

void Application::InitVulkan()
{
    m_instance.Create();
    m_log.CreateDebugMessenger();
}

void Application::MainLoop()
{
    while (!m_window.ShouldClose()) {
        glfwPollEvents();
    }
}
