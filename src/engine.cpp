#include "engine.h"

GraphicsEngine::GraphicsEngine(int width, int height, GLFWwindow* window)
{
    this->window = window;
    this->width = width;
    this->height = height;

    m_instance.CreateInstance();
    m_logger.CreateDebugMessenger();
    m_instance.CreateSurface(window);

    m_device.CreateDevice();

    m_swapchain.CreateSwapchain(window);

    m_pipeline.CreatePipeline();

    m_framebuffer.CreateFramebuffer();
    m_command.CreateCommandPool();
    for (auto& frame : swapchainDetails.frames) {
        m_command.CreateCommandBuffer(frame.commandBuffer);
    }

    m_sync.maxFramesInFlight = static_cast<int>(swapchainDetails.frames.size());
    m_sync.frameNumber = 0;

    for (auto& frame : swapchainDetails.frames) {
        frame.inFlight = m_sync.MakeFence();
        frame.imageAvailable = m_sync.MakeSemaphore();
        frame.renderFinished = m_sync.MakeSemaphore();
    }
}

void GraphicsEngine::Prepare(Scene* scene)
{
    std::vector<float>& vertices = scene->m_triangleMesh->vertices;

    BufferInput stagingBufferInput;
    stagingBufferInput.size = sizeof(float) * vertices.size();
    stagingBufferInput.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferInput.properties = vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent;

    Buffer& stagingBuffer = scene->m_triangleMesh->m_stagingBuffer;
    stagingBuffer = CreateBuffer(stagingBufferInput);

    void* memoryLocation = device.mapMemory(stagingBuffer.memory, 0, stagingBufferInput.size);
    memcpy(memoryLocation, vertices.data(), stagingBufferInput.size);
    device.unmapMemory(stagingBuffer.memory);

    BufferInput vertexBufferInput;
    vertexBufferInput.size = stagingBufferInput.size;
    vertexBufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    vertexBufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    Buffer& vertexBuffer = scene->m_triangleMesh->m_vertexBuffer;
    vertexBuffer = CreateBuffer(vertexBufferInput);

    Command copyCommand;
    copyCommand.CreateCommandPool();

    vk::CommandBuffer copyCommandBuffer;
    copyCommand.CreateCommandBuffer(copyCommandBuffer);

    copyCommand.RecordCopyCommands(copyCommandBuffer, stagingBuffer.buffer, vertexBuffer.buffer, stagingBufferInput.size);

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copyCommandBuffer;

    graphicsQueue.submit(submitInfo);
    graphicsQueue.waitIdle();
}

void GraphicsEngine::Render(Scene* scene)
{
    auto resultWaitFence = device.waitForFences(1, &swapchainDetails.frames[m_sync.frameNumber].inFlight, VK_TRUE, UINT64_MAX);
    auto resultResetFence = device.resetFences(1, &swapchainDetails.frames[m_sync.frameNumber].inFlight);

    uint32_t imageIndex;
    auto acquiredImage = device.acquireNextImageKHR(swapchainDetails.swapchain, UINT64_MAX, swapchainDetails.frames[m_sync.frameNumber].imageAvailable, nullptr);
    if (acquiredImage.result == vk::Result::eErrorOutOfDateKHR ||
        acquiredImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        return;
    }
    imageIndex = acquiredImage.value;

    vk::CommandBuffer commandBuffer = swapchainDetails.frames[m_sync.frameNumber].commandBuffer;

    commandBuffer.reset();
    m_command.RecordDrawCommands(commandBuffer, imageIndex, scene);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { swapchainDetails.frames[m_sync.frameNumber].imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vk::Semaphore signalSemaphores[] = { swapchainDetails.frames[m_sync.frameNumber].renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    graphicsQueue.submit(submitInfo, swapchainDetails.frames[m_sync.frameNumber].inFlight);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapchains[] = { swapchainDetails.swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vk::Result resultPresent;
    try {
        resultPresent = presentQueue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError error) {
        RecreateSwapchain();
        return;
    }

    m_sync.frameNumber = (m_sync.frameNumber + 1) % m_sync.maxFramesInFlight;
}

void GraphicsEngine::RecreateSwapchain()
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    device.waitIdle();
    for (auto& frame : swapchainDetails.frames) {
        device.freeCommandBuffers(m_command.commandPool, frame.commandBuffer);
    }
    m_swapchain.DestroySwapchain();
    device.destroyCommandPool(m_command.commandPool);

    m_swapchain.CreateSwapchain(window);
    m_framebuffer.CreateFramebuffer();

    for (auto& frame : swapchainDetails.frames) {
        frame.inFlight = m_sync.MakeFence();
        frame.imageAvailable = m_sync.MakeSemaphore();
        frame.renderFinished = m_sync.MakeSemaphore();
    }

    m_command.CreateCommandPool();
    for (auto& frame : swapchainDetails.frames) {
        m_command.CreateCommandBuffer(frame.commandBuffer);
    }
}

GraphicsEngine::~GraphicsEngine()
{
    device.waitIdle();
}
