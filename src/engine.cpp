#include "engine.h"

GraphicsEngine::GraphicsEngine()
{
    swapchain_.CreateSwapchain();
    pipeline_.CreatePipeline();
    swapchain_.CreateFrameBuffer(pipeline_.GetHandle().renderPass);

    command_.CreateCommandPool("swapchain frames");
    for (auto& frame : Swapchain::GetDetail().frames) {
        command_.AllocateCommandBuffer(frame.commandBuffer);
    }

    maxFrameNumber_ = static_cast<int>(Swapchain::GetDetail().frames.size());
    frameIndex_ = 0;

    swapchain_.PrepareFrames();

    pipeline_.CreateDescriptorPool();
    for (auto& frame : Swapchain::GetDetail().frames) {
        pipeline_.AllocateDescriptorSet(frame.descriptorSets);
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

void GraphicsEngine::Render(std::unique_ptr<Scene>& scene)
{
    if (Device::GetHandle().device.waitForFences(1, &Swapchain::GetDetail().frames[frameIndex_].inFlight, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
        spdlog::error("failed to wait for fences");
    }

    auto waitFrameImage = Device::GetHandle().device.acquireNextImageKHR(Swapchain::GetHandle(), UINT64_MAX, Swapchain::GetDetail().frames[frameIndex_].imageAvailable, nullptr);

    if (waitFrameImage.result == vk::Result::eErrorOutOfDateKHR ||
        waitFrameImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    if (Device::GetHandle().device.resetFences(1, &Swapchain::GetDetail().frames[frameIndex_].inFlight) != vk::Result::eSuccess) {
        spdlog::error("failed to reset fences");
    }

    scene->Update(waitFrameImage.value);

    vk::CommandBuffer commandBuffer = Swapchain::GetDetail().frames[frameIndex_].commandBuffer;

    commandBuffer.reset();
    command_.RecordDrawCommands(pipeline_, commandBuffer, waitFrameImage.value, scene->meshes, scene->uboDataDynamic_.alignment, imDrawData_);

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { Swapchain::GetDetail().frames[frameIndex_].imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vk::Semaphore signalSemaphores[] = { Swapchain::GetDetail().frames[frameIndex_].renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    Queue::GetHandle().graphicsQueue.submit(submitInfo, Swapchain::GetDetail().frames[frameIndex_].inFlight);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapchains[] = { Swapchain::GetHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &waitFrameImage.value;

    if (Queue::GetHandle().presentQueue.presentKHR(presentInfo) == vk::Result::eErrorOutOfDateKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        return;
    }

    frameIndex_ = (frameIndex_ + 1) % maxFrameNumber_;
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
        Device::GetHandle().device.freeCommandBuffers(command_.commandPool, frame.commandBuffer);
    }
    swapchain_.DestroySwapchain();
    Device::GetHandle().device.destroyCommandPool(command_.commandPool);

    swapchain_.CreateSwapchain();
    swapchain_.CreateFrameBuffer(pipeline_.GetHandle().renderPass);

    swapchain_.PrepareFrames();

    command_.CreateCommandPool("new swapchain frames");
    for (auto& frame : Swapchain::GetDetail().frames) {
        command_.AllocateCommandBuffer(frame.commandBuffer);
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
