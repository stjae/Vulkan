#include "engine.h"

GraphicsEngine::GraphicsEngine()
{
    swapchain_.CreateSwapchain();
    pipeline_.CreatePipeline(swapchain_.GetDescriptorSetLayouts());
    swapchain_.CreateFrameBuffer(pipeline_.GetHandle().renderPass);

    frameIndex_ = 0;
    swapchain_.PrepareFrames();
}

void GraphicsEngine::InitSwapchainImages()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    for (auto& frame : swapchain_.frames_) {
        if (frame.command.commandBuffers_.back().begin(&beginInfo) != vk::Result::eSuccess) {
            spdlog::error("failed to begin command buffer");
        }

        vk::ImageMemoryBarrier barrier;

        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();
        barrier.dstQueueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();
        barrier.image = frame.swapchainImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        frame.command.commandBuffers_.back().pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, {}, 0, nullptr, 0, nullptr, 1, &barrier);

        frame.command.commandBuffers_.back().end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.command.commandBuffers_.back();

        if (Queue::GetHandle().graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess) {
            spdlog::error("failed to get graphcis queue");
        }
        Device::GetHandle().device.waitIdle();
    }
}

void GraphicsEngine::Render(std::unique_ptr<Scene>& scene)
{
    if (Device::GetHandle().device.waitForFences(1, &swapchain_.frames_[frameIndex_].inFlight, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
        spdlog::error("failed to wait for fences");
    }

    auto waitFrameImage = Device::GetHandle().device.acquireNextImageKHR(Swapchain::Get().swapchain, UINT64_MAX, swapchain_.frames_[frameIndex_].imageAvailable, nullptr);
    int nextFrameIndex = (int)waitFrameImage.value;

    if (waitFrameImage.result == vk::Result::eErrorOutOfDateKHR ||
        waitFrameImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    if (Device::GetHandle().device.resetFences(1, &swapchain_.frames_[frameIndex_].inFlight) != vk::Result::eSuccess) {
        spdlog::error("failed to reset fences");
    }

    scene->Update(nextFrameIndex, swapchain_.frames_);

    swapchain_.RecordDrawCommand(pipeline_, nextFrameIndex, scene->meshes, scene->uboDataDynamic_.alignment, imDrawData_);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { swapchain_.frames_[frameIndex_].imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = swapchain_.frames_[nextFrameIndex].command.commandBuffers_.data();
    vk::Semaphore signalSemaphores[] = { swapchain_.frames_[frameIndex_].renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    Queue::GetHandle().graphicsQueue.submit(submitInfo, swapchain_.frames_[frameIndex_].inFlight);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapchains[] = { Swapchain::Get().swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &waitFrameImage.value;

    if (Queue::GetHandle().presentQueue.presentKHR(presentInfo) == vk::Result::eErrorOutOfDateKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    frameIndex_ = (frameIndex_ + 1) % Swapchain::Get().frameCount;
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
    for (auto& frame : swapchain_.frames_) {
        Device::GetHandle().device.freeCommandBuffers(frame.command.commandPool_, frame.command.commandBuffers_);
        Device::GetHandle().device.destroyCommandPool(frame.command.commandPool_);
    }
    swapchain_.DestroySwapchain();

    swapchain_.CreateSwapchain();
    swapchain_.CreateFrameBuffer(pipeline_.GetHandle().renderPass);

    swapchain_.PrepareFrames();

    for (auto& frame : swapchain_.frames_) {
        //        frame.ReallocateCommandBuffers();
    }
}

void GraphicsEngine::SetupGui()
{
    imgui_.Setup(pipeline_);
}

void GraphicsEngine::DrawGui(std::unique_ptr<Scene>& scene)
{
    imgui_.Draw(scene, frameIndex_);
    imDrawData_ = ImGui::GetDrawData();
}

GraphicsEngine::~GraphicsEngine()
{
    Device::GetHandle().device.waitIdle();
}
