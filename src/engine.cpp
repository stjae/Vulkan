#include "engine.h"

GraphicsEngine::GraphicsEngine(std::shared_ptr<Scene>& scene)
{
    scene_ = scene;

    swapchain.CreateSwapchain();
    pipeline.CreatePipeline();
    swapchain.CreateFrameBuffer(pipeline.GetHandle().renderPass);

    command.CreateCommandPool("swapchain frames");
    for (auto& frame : Swapchain::GetDetail().frames) {
        command.AllocateCommandBuffer(frame.commandBuffer);
    }

    maxFrameNumber = static_cast<int>(Swapchain::GetDetail().frames.size());
    frameIndex = 0;

    swapchain.PrepareFrames();

    pipeline.CreateDescriptorPool();
    for (auto& frame : Swapchain::GetDetail().frames) {
        pipeline.AllocateDescriptorSet(frame.descriptorSets);
    }
}

void GraphicsEngine::InitSwapchainImages()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    for (auto& frame : Swapchain::GetDetail().frames) {
        if (frame.commandBuffer.begin(&beginInfo) != vk::Result::eSuccess) {
            spdlog::error("failed to begin command buffer");
        }

        vk::ImageMemoryBarrier barrier;

        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();
        barrier.dstQueueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();
        barrier.image = frame.swapchainVkImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        frame.commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, {}, 0, nullptr, 0, nullptr, 1, &barrier);

        frame.commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.commandBuffer;

        if (Queue::GetHandle().graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess) {
            spdlog::error("failed to get graphcis queue");
        }
        Device::GetHandle().device.waitIdle();
    }
}

void GraphicsEngine::Render()
{
    auto resultWaitFence = Device::GetHandle().device.waitForFences(1, &Swapchain::GetDetail().frames[frameIndex].inFlight, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    auto acquiredImage = Device::GetHandle().device.acquireNextImageKHR(Swapchain::GetSwapchain(), UINT64_MAX, Swapchain::GetDetail().frames[frameIndex].imageAvailable, nullptr);

    if (acquiredImage.result == vk::Result::eErrorOutOfDateKHR ||
        acquiredImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    auto resultResetFence = Device::GetHandle().device.resetFences(1, &Swapchain::GetDetail().frames[frameIndex].inFlight);

    imageIndex = acquiredImage.value;

    scene_.lock()->Update(imageIndex);

    vk::CommandBuffer commandBuffer = Swapchain::GetDetail().frames[frameIndex].commandBuffer;

    commandBuffer.reset();
    command.RecordDrawCommands(pipeline, commandBuffer, imageIndex, scene_.lock()->meshes, imDrawData);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { Swapchain::GetDetail().frames[frameIndex].imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vk::Semaphore signalSemaphores[] = { Swapchain::GetDetail().frames[frameIndex].renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    Queue::GetHandle().graphicsQueue.submit(submitInfo, Swapchain::GetDetail().frames[frameIndex].inFlight);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapchains[] = { Swapchain::GetSwapchain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vk::Result resultPresent;
    try {
        resultPresent = Queue::GetHandle().presentQueue.presentKHR(presentInfo);
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
        glfwGetFramebufferSize(*Window::GetWindow(), &width, &height);
        glfwWaitEvents();
    }

    Device::GetHandle().device.waitIdle();
    for (auto& frame : Swapchain::GetDetail().frames) {
        Device::GetHandle().device.freeCommandBuffers(command.commandPool, frame.commandBuffer);
    }
    swapchain.DestroySwapchain();
    Device::GetHandle().device.destroyCommandPool(command.commandPool);

    swapchain.CreateSwapchain();
    swapchain.CreateFrameBuffer(pipeline.GetHandle().renderPass);

    swapchain.PrepareFrames();

    command.CreateCommandPool("new swapchain frames");
    for (auto& frame : Swapchain::GetDetail().frames) {
        command.AllocateCommandBuffer(frame.commandBuffer);
    }
}

void GraphicsEngine::SetupGui()
{
    imgui.Setup(scene_, pipeline);
}

void GraphicsEngine::DrawGui()
{
    imgui.Draw();
    imDrawData = ImGui::GetDrawData();
}

GraphicsEngine::~GraphicsEngine()
{
    Device::GetHandle().device.waitIdle();
}
