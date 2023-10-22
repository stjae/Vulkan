#include "engine.h"

GraphicsEngine::GraphicsEngine(int width, int height, GLFWwindow* window)
{
    m_window = window;
    m_width = width;
    m_height = height;

    m_instance.CreateInstance();
    m_logger.CreateDebugMessenger();
    m_instance.CreateSurface(window);

    m_device.CreateDevice();

    m_swapchain.CreateSwapchain(window);
    m_pipeline.CreatePipeline();
    m_swapchain.CreateFrameBuffer();

    m_command.CreateCommandPool();
    for (auto& frame : swapchainDetails.frames) {
        m_command.CreateCommandBuffer(frame.commandBuffer);
    }

    m_maxFrameNumber = static_cast<int>(swapchainDetails.frames.size());
    m_frameIndex = 0;

    m_pipeline.CreateDescriptorPool();

    m_swapchain.PrepareFrames();

    for (auto& frame : swapchainDetails.frames) {
        m_pipeline.AllocateDescriptorSet(frame.descriptorSet);
    }
}

void GraphicsEngine::UpdateFrame(uint32_t imageIndex)
{
    glm::vec3 eye = { 0.0f, 0.0f, 2.0f };
    glm::vec3 center = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    glm::mat4 view = glm::lookAt(eye, center, up);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchainDetails.extent.width) / static_cast<float>(swapchainDetails.extent.height), 0.1f, 10.0f);
    proj[1][1] *= -1;

    swapchainDetails.frames[imageIndex].cameraData.view = view;
    swapchainDetails.frames[imageIndex].cameraData.proj = proj;
    swapchainDetails.frames[imageIndex].cameraData.viewProj = proj * view;
    memcpy(swapchainDetails.frames[imageIndex].cameraDataMemoryLocation, &(swapchainDetails.frames[imageIndex].cameraData), sizeof(UBO));

    swapchainDetails.frames[imageIndex].WriteDescriptorSet();
}

void GraphicsEngine::Prepare(Scene* scene)
{
    Command copyVertexCommand;
    copyVertexCommand.CreateCommandPool();

    vk::CommandBuffer copyVertexCommandBuffer;
    copyVertexCommand.CreateCommandBuffer(copyVertexCommandBuffer);
    for (auto mesh : scene->m_meshes) {
        copyVertexCommand.RecordCopyCommands(copyVertexCommandBuffer, mesh->m_vertexStagingBuffer.buffer, mesh->m_vertexBuffer.buffer, sizeof(mesh->m_vertices[0]) * mesh->m_vertices.size());
    }

    Command copyIndexCommand;
    copyIndexCommand.CreateCommandPool();

    vk::CommandBuffer copyIndexCommandBuffer;
    copyIndexCommand.CreateCommandBuffer(copyIndexCommandBuffer);
    for (auto mesh : scene->m_meshes) {
        copyIndexCommand.RecordCopyCommands(copyIndexCommandBuffer, mesh->m_indexStagingBuffer.buffer, mesh->m_indexBuffer.buffer, sizeof(mesh->m_indices[0]) * mesh->m_indices.size());
    }

    std::vector<vk::CommandBuffer> buffers{ copyVertexCommandBuffer, copyIndexCommandBuffer };

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
    submitInfo.pCommandBuffers = buffers.data();

    graphicsQueue.submit(submitInfo);
    graphicsQueue.waitIdle();
}

void GraphicsEngine::Render(Scene* scene)
{
    auto resultWaitFence = device.waitForFences(1, &swapchainDetails.frames[m_frameIndex].inFlight, VK_TRUE, UINT64_MAX);
    auto resultResetFence = device.resetFences(1, &swapchainDetails.frames[m_frameIndex].inFlight);

    uint32_t imageIndex;
    auto acquiredImage = device.acquireNextImageKHR(swapchainDetails.swapchain, UINT64_MAX, swapchainDetails.frames[m_frameIndex].imageAvailable, nullptr);

    if (acquiredImage.result == vk::Result::eErrorOutOfDateKHR ||
        acquiredImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        return;
    }

    imageIndex = acquiredImage.value;

    UpdateFrame(imageIndex);

    vk::CommandBuffer commandBuffer = swapchainDetails.frames[m_frameIndex].commandBuffer;

    commandBuffer.reset();
    m_command.RecordDrawCommands(commandBuffer, imageIndex, scene);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { swapchainDetails.frames[m_frameIndex].imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vk::Semaphore signalSemaphores[] = { swapchainDetails.frames[m_frameIndex].renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    graphicsQueue.submit(submitInfo, swapchainDetails.frames[m_frameIndex].inFlight);

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

    m_frameIndex = (m_frameIndex + 1) % m_maxFrameNumber;
}

void GraphicsEngine::RecreateSwapchain()
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    device.waitIdle();
    for (auto& frame : swapchainDetails.frames) {
        device.freeCommandBuffers(m_command.m_commandPool, frame.commandBuffer);
    }
    m_swapchain.DestroySwapchain();
    device.destroyCommandPool(m_command.m_commandPool);

    m_swapchain.CreateSwapchain(m_window);
    m_swapchain.CreateFrameBuffer();

    m_swapchain.PrepareFrames();

    m_command.CreateCommandPool();
    for (auto& frame : swapchainDetails.frames) {
        m_command.CreateCommandBuffer(frame.commandBuffer);
    }
}

GraphicsEngine::~GraphicsEngine()
{
    device.waitIdle();
}
