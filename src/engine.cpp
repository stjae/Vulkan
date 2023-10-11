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

void GraphicsEngine::Render()
{
    device.waitForFences(1, &m_sync.inFlightFence, VK_TRUE, UINT64_MAX);
    device.resetFences(1, &m_sync.inFlightFence);

    uint32_t imageIndex{ device.acquireNextImageKHR(swapchainDetails.swapchain, UINT64_MAX, m_sync.imageAvailable, nullptr).value };

    vk::CommandBuffer commandBuffer = swapchainDetails.frames[imageIndex].commandBuffer;

    commandBuffer.reset();
    m_command.RecordDrawCommands(commandBuffer, imageIndex);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { m_sync.imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphoreCount(1);
    submitInfo.setPWaitSemaphores(waitSemaphores);
    submitInfo.setPWaitDstStageMask(waitStage);
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&commandBuffer);
    vk::Semaphore signalSemaphores[] = { m_sync.renderFinished };
    submitInfo.setSignalSemaphoreCount(1);
    submitInfo.setPSignalSemaphores(signalSemaphores);

    graphicsQueue.submit(submitInfo, m_sync.inFlightFence);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphoreCount(1);
    presentInfo.setPWaitSemaphores(signalSemaphores);
    vk::SwapchainKHR swapchains[] = { swapchainDetails.swapchain };
    presentInfo.setSwapchainCount(1);
    presentInfo.setPSwapchains(swapchains);
    presentInfo.setPImageIndices(&imageIndex);

    presentQueue.presentKHR(presentInfo);
}

GraphicsEngine::~GraphicsEngine()
{
    device.waitIdle();
}