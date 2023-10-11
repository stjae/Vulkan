#include "engine.h"

GraphicsEngine::GraphicsEngine(int width, int height, GLFWwindow* window)
{
    this->window = window;
    m_instance.CreateInstance();
    m_logger.CreateDebugMessenger();
    m_instance.CreateSurface(window);

    m_device.PickPhysicalDevice();
    m_device.FindQueueFamilies();
    m_device.CreateDevice();

    m_swapchain.QuerySwapchainSupportDetails();
    m_swapchain.CreateSwapchain(window);

    m_pipeline.CreatePipeline();

    m_framebuffer.CreateFramebuffer();
    m_command.CreateCommandPool();
    m_command.CreateCommandBuffer();

    m_sync.inFlightFence = m_sync.CreateVkFence();
    m_sync.imageAvailable = m_sync.CreateVkSemaphore();
    m_sync.renderFinished = m_sync.CreateVkSemaphore();
}
