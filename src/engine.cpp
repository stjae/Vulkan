#include "engine.h"

GraphicsEngine::GraphicsEngine(int width, int height, GLFWwindow* window, std::unique_ptr<Scene>& scene)
    : device(window), swapchain(device.vkPhysicalDevice, device.vkDevice, device.instance.vkSurface, pipeline.vkRenderPass),
      pipeline(device.vkDevice, swapchain.detail, scene), command(device)
{
    this->window = window;
    this->width = width;
    this->height = height;

    swapchain.CreateSwapchain(this->window, device);
    pipeline.CreatePipeline();
    swapchain.CreateFrameBuffer();

    command.CreateCommandPool();
    for (auto& frame : swapchain.detail.frames) {
        command.CreateCommandBuffer(frame.commandBuffer);
    }

    maxFrameNumber = static_cast<int>(swapchain.detail.frames.size());
    frameIndex = 0;

    swapchain.PrepareFrames();

    pipeline.CreateDescriptorPool();
    for (auto& frame : swapchain.detail.frames) {
        pipeline.descriptor.AllocateSet(frame.descriptorSets);
    }
}

void GraphicsEngine::InitSwapchainImages()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    for (auto& frame : swapchain.detail.frames) {
        frame.commandBuffer.begin(&beginInfo);

        vk::ImageMemoryBarrier barrier;

        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();
        barrier.dstQueueFamilyIndex = device.queueFamilyIndices.graphicsFamily.value();
        barrier.image = frame.image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        frame.commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
                                            vk::DependencyFlagBits::eByRegion,
                                            0, nullptr, 0, nullptr, 1, &barrier);

        frame.commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.commandBuffer;

        device.vkGraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
        device.vkDevice.waitIdle();
    }
}

void GraphicsEngine::UpdateFrame(uint32_t imageIndex, Camera& camera)
{
    uboData.model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
    uboData.view = glm::lookAt(camera.pos, camera.at, camera.up);
    uboData.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchain.detail.extent.width) / static_cast<float>(swapchain.detail.extent.height), 0.1f, 100.0f);
    uboData.proj[1][1] *= -1;

    swapchain.detail.frames[imageIndex].uboData.model = uboData.model;
    swapchain.detail.frames[imageIndex].uboData.view = uboData.view;
    swapchain.detail.frames[imageIndex].uboData.proj = uboData.proj;
    memcpy(swapchain.detail.frames[imageIndex].uboDataMemoryLocation, &(swapchain.detail.frames[imageIndex].uboData), sizeof(UBO));

    swapchain.detail.frames[imageIndex].WriteDescriptorSet(device.vkDevice);
}

void GraphicsEngine::Prepare(std::unique_ptr<Scene>& scene)
{
    scene->CreateResource(device.vkPhysicalDevice, device.vkDevice);

    Command copyVertexCommand(device);
    copyVertexCommand.CreateCommandPool();

    vk::CommandBuffer copyVertexCommandBuffer;
    copyVertexCommand.CreateCommandBuffer(copyVertexCommandBuffer);
    for (auto& mesh : scene->meshes) {
        copyVertexCommand.RecordCopyCommands(copyVertexCommandBuffer, mesh->vertexStagingBuffer->vkBuffer, mesh->vertexBuffer->vkBuffer, sizeof(mesh->vertices[0]) * mesh->vertices.size());
    }

    Command copyIndexCommand(device);
    copyIndexCommand.CreateCommandPool();

    vk::CommandBuffer copyIndexCommandBuffer;
    copyIndexCommand.CreateCommandBuffer(copyIndexCommandBuffer);
    for (auto& mesh : scene->meshes) {
        copyIndexCommand.RecordCopyCommands(copyIndexCommandBuffer, mesh->indexStagingBuffer->vkBuffer, mesh->indexBuffer->vkBuffer, sizeof(mesh->indices[0]) * mesh->indices.size());
    }

    std::vector<vk::CommandBuffer> buffers{ copyVertexCommandBuffer, copyIndexCommandBuffer };

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
    submitInfo.pCommandBuffers = buffers.data();

    device.vkGraphicsQueue.submit(submitInfo);
    device.vkGraphicsQueue.waitIdle();
}

void GraphicsEngine::Render(std::unique_ptr<Scene>& scene, ImDrawData* imDrawData, Camera& camera)
{
    auto resultWaitFence = device.vkDevice.waitForFences(1, &swapchain.detail.frames[frameIndex].inFlight, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    auto acquiredImage = device.vkDevice.acquireNextImageKHR(swapchain.vkSwapchain, UINT64_MAX, swapchain.detail.frames[frameIndex].imageAvailable, nullptr);

    if (acquiredImage.result == vk::Result::eErrorOutOfDateKHR ||
        acquiredImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    auto resultResetFence = device.vkDevice.resetFences(1, &swapchain.detail.frames[frameIndex].inFlight);

    imageIndex = acquiredImage.value;

    UpdateFrame(imageIndex, camera);

    vk::CommandBuffer commandBuffer = swapchain.detail.frames[frameIndex].commandBuffer;

    commandBuffer.reset();
    command.RecordDrawCommands(pipeline, commandBuffer, imageIndex, scene, imDrawData);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { swapchain.detail.frames[frameIndex].imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vk::Semaphore signalSemaphores[] = { swapchain.detail.frames[frameIndex].renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    device.vkGraphicsQueue.submit(submitInfo, swapchain.detail.frames[frameIndex].inFlight);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapchains[] = { swapchain.vkSwapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vk::Result resultPresent;
    try {
        resultPresent = device.vkPresentQueue.presentKHR(presentInfo);
    } catch (vk::OutOfDateKHRError error) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    frameIndex = (frameIndex + 1) % maxFrameNumber;
}

void GraphicsEngine::RecreateSwapchain()
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    device.vkDevice.waitIdle();
    for (auto& frame : swapchain.detail.frames) {
        device.vkDevice.freeCommandBuffers(command.commandPool, frame.commandBuffer);
    }
    swapchain.DestroySwapchain();
    device.vkDevice.destroyCommandPool(command.commandPool);

    swapchain.CreateSwapchain(window, device);
    swapchain.CreateFrameBuffer();

    swapchain.PrepareFrames();

    command.CreateCommandPool();
    for (auto& frame : swapchain.detail.frames) {
        command.CreateCommandBuffer(frame.commandBuffer);
    }
}

GraphicsEngine::~GraphicsEngine()
{
    device.vkDevice.waitIdle();
}
