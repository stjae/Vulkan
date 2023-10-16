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

    m_sync.maxFramesInFlight = static_cast<int>(swapchainDetails.frames.size());
    m_sync.frameNumber = 0;

    for (auto& frame : swapchainDetails.frames) {
        frame.inFlight = m_sync.MakeFence();
        frame.imageAvailable = m_sync.MakeSemaphore();
        frame.renderFinished = m_sync.MakeSemaphore();
    }
}

void GraphicsEngine::Render(Scene* scene)
{
    auto resultWaitFence = device.waitForFences(1, &swapchainDetails.frames[m_sync.frameNumber].inFlight, VK_TRUE, UINT64_MAX);
    auto resultResetFence = device.resetFences(1, &swapchainDetails.frames[m_sync.frameNumber].inFlight);

    uint32_t imageIndex{ device.acquireNextImageKHR(swapchainDetails.swapchain, UINT64_MAX, swapchainDetails.frames[m_sync.frameNumber].imageAvailable, nullptr).value };

    vk::CommandBuffer commandBuffer = swapchainDetails.frames[m_sync.frameNumber].commandBuffer;

    commandBuffer.reset();
    m_command.RecordDrawCommands(commandBuffer, imageIndex, scene);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { swapchainDetails.frames[m_sync.frameNumber].imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphoreCount(1);
    submitInfo.setPWaitSemaphores(waitSemaphores);
    submitInfo.setPWaitDstStageMask(waitStage);
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&commandBuffer);
    vk::Semaphore signalSemaphores[] = { swapchainDetails.frames[m_sync.frameNumber].renderFinished };
    submitInfo.setSignalSemaphoreCount(1);
    submitInfo.setPSignalSemaphores(signalSemaphores);

    graphicsQueue.submit(submitInfo, swapchainDetails.frames[m_sync.frameNumber].inFlight);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphoreCount(1);
    presentInfo.setPWaitSemaphores(signalSemaphores);
    vk::SwapchainKHR swapchains[] = { swapchainDetails.swapchain };
    presentInfo.setSwapchainCount(1);
    presentInfo.setPSwapchains(swapchains);
    presentInfo.setPImageIndices(&imageIndex);

    auto resultPresent = presentQueue.presentKHR(presentInfo);

    m_sync.frameNumber = (m_sync.frameNumber + 1) % m_sync.maxFramesInFlight;
}

GraphicsEngine::~GraphicsEngine()
{
    device.waitIdle();
}