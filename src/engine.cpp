#include "engine.h"

GraphicsEngine::GraphicsEngine(int width, int height, GLFWwindow* window, std::unique_ptr<Scene>& scene)
    : device(window), swapchain(device, pipeline.vkRenderPass),
      pipeline(device.vkDevice, swapchain.detail), command(device)
{
    this->window = window;
    this->width = width;
    this->height = height;

    swapchain.CreateSwapchain(this->window, device);
    pipeline.CreatePipeline();
    swapchain.CreateFrameBuffer();

    command.CreateCommandPool("swapchain frames");
    for (auto& frame : swapchain.detail.frames) {
        command.AllocateCommandBuffer(frame.commandBuffer);
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
        barrier.image = frame.swapchainVkImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        frame.commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, {}, 0, nullptr, 0, nullptr, 1, &barrier);

        frame.commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.commandBuffer;

        device.vkGraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
        device.vkDevice.waitIdle();
    }
}

void GraphicsEngine::Render(std::unique_ptr<Scene>& scene, ImDrawData* imDrawData)
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

    scene->Update(imageIndex, swapchain, device.vkDevice);

    vk::CommandBuffer commandBuffer = swapchain.detail.frames[frameIndex].commandBuffer;

    commandBuffer.reset();
    command.RecordDrawCommands(pipeline, commandBuffer, imageIndex, scene->meshes_, imDrawData);

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

    command.CreateCommandPool("new swapchain frames");
    for (auto& frame : swapchain.detail.frames) {
        command.AllocateCommandBuffer(frame.commandBuffer);
    }
}

GraphicsEngine::~GraphicsEngine()
{
    device.vkDevice.waitIdle();
}
