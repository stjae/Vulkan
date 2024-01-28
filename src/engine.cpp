#include "engine.h"

void Engine::InitSwapchainImages()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    for (auto& frame : swapchain_.frames) {
        frame.commandBuffer.begin(&beginInfo);

        vk::ImageMemoryBarrier barrier;

        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = Device::GetBundle().graphicsFamilyIndex.value();
        barrier.dstQueueFamilyIndex = Device::GetBundle().graphicsFamilyIndex.value();
        barrier.image = frame.swapchainImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        frame.commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, {}, 0, nullptr, 0, nullptr, 1, &barrier);

        frame.commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.commandBuffer;

        Device::GetBundle().graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);

        Device::GetBundle().device.waitIdle();
    }
}

void Engine::SetUp()
{
    scene_ = std::make_unique<Scene>();
    imgui_.Setup(swapchain_.GetRenderPass(), viewport_);
}

void Engine::UpdateScene()
{
    scene_->Update(frameIndex_, viewport_.frames);
}

void Engine::Render()
{
    Device::GetBundle().device.waitForFences(1, &swapchain_.frames[frameIndex_].inFlight, VK_TRUE, UINT64_MAX);

    auto waitFrameImage = Device::GetBundle().device.acquireNextImageKHR(Swapchain::GetBundle().swapchain, UINT64_MAX, swapchain_.frames[frameIndex_].imageAvailable, nullptr);

    if (IsSwapchainOutOfDate(waitFrameImage))
        return;

    Device::GetBundle().device.resetFences(1, &swapchain_.frames[frameIndex_].inFlight);

    swapchain_.Draw(frameIndex_, imDrawData_);
    viewport_.Draw(frameIndex_, scene_->meshes, scene_->uboDataDynamic.alignment);
    swapchain_.Submit(frameIndex_);
    swapchain_.Present(frameIndex_, waitFrameImage);

    /*
        if (Queue::GetBundle().presentQueue.presentKHR(presentInfo) == vk::Result::eErrorOutOfDateKHR) {
            RecreateSwapchain();
            InitSwapchainImages();

            return;
        }
    */

    frameIndex_ = (frameIndex_ + 1) % Swapchain::GetBundle().frameCount;
}

bool Engine::IsSwapchainOutOfDate(const vk::ResultValue<unsigned int>& waitFrameImage)
{
    if (waitFrameImage.result == vk::Result::eErrorOutOfDateKHR || waitFrameImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        viewport_.outDated = true;
        viewport_.RecreateViewportImages();
        imgui_.RecreateViewportDescriptorSets(viewport_);
        return true;
    } else
        return false;
}

void Engine::RecreateSwapchain()
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(*Window::GetWindow(), &width, &height);
        glfwWaitEvents();
    }

    Device::GetBundle().device.waitIdle();
    for (auto& frame : swapchain_.frames) {
        Device::GetBundle().device.freeCommandBuffers(frame.commandPool, frame.commandBuffer);
        Device::GetBundle().device.destroyCommandPool(frame.commandPool);
    }
    swapchain_.Destroy();

    swapchain_.CreateSwapchain();
    swapchain_.CreateFrameBuffer();

    swapchain_.PrepareFrames();
}

void Engine::DrawUI()
{
    imgui_.Draw(*scene_, viewport_, frameIndex_);
    imDrawData_ = ImGui::GetDrawData();
}

Engine::~Engine()
{
    Device::GetBundle().device.waitIdle();
}
