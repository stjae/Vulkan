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
    m_instance.CreateInstance();
    m_logger.CreateDebugMessenger();
    m_instance.CreateSurface();

    m_device.PickPhysicalDevice();
    m_device.FindQueueFamilies();
    m_device.CreateDevice();

    m_swapchain.QuerySwapchainSupportDetails();
    m_swapchain.CreateSwapchain();

    m_pipeline.CreatePipeline();

    m_framebuffer.CreateFramebuffer();
    m_command.CreateCommandPool();
    m_command.CreateCommandBuffer();
}

void Application::MainLoop()
{
    while (!m_window.ShouldClose()) {
        glfwPollEvents();
    }
}
