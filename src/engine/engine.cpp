#include "engine.h"

Engine::Engine()
{
    InitSwapchainImages();
    scene_ = std::make_unique<Scene>();

    Command::CreateCommandPool(commandPool_);
    Command::AllocateCommandBuffer(commandPool_, commandBuffer_);

    SetUpDescriptors();
    CreatePipeline();

    imgui_.Setup(swapchain_.GetRenderPass(), viewport_, *scene_);
}

void Engine::InitSwapchainImages()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    for (auto& frame : swapchain_.frames) {
        Command::Begin(frame.commandBuffer);
        Command::SetImageMemoryBarrier(frame.commandBuffer, frame.swapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, {}, {}, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe);
        frame.commandBuffer.end();
        Command::Submit(&frame.commandBuffer, 1);

        Device::GetBundle().device.waitIdle();
    }
}

void Engine::Render()
{
    scene_->Update();

    Device::GetBundle().device.waitForFences(1, &swapchain_.frames[frameIndex_].inFlight, VK_TRUE, UINT64_MAX);

    auto waitFrameImage = Device::GetBundle().device.acquireNextImageKHR(Swapchain::GetBundle().swapchain, UINT64_MAX, swapchain_.frames[frameIndex_].imageAvailable, nullptr);

    if (waitFrameImage.result == vk::Result::eErrorOutOfDateKHR || waitFrameImage.result == vk::Result::eSuboptimalKHR || Window::resized) {
        Window::resized = false;
        UpdateSwapchain();
        return;
    }

    Device::GetBundle().device.resetFences(1, &swapchain_.frames[frameIndex_].inFlight);

    swapchain_.Draw(frameIndex_, UI::imDrawData);
    viewport_.Draw(scene_->GetMeshes());
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && viewport_.isMouseHovered) {
        scene_->SelectByColorID(viewport_);
    }
    imgui_.AcceptDragDrop(viewport_, *scene_, frameIndex_);
    swapchain_.Submit(frameIndex_);
    swapchain_.Present(frameIndex_, waitFrameImage);

    frameIndex_ = (frameIndex_ + 1) % Swapchain::GetBundle().frameImageCount;
}

void Engine::UpdateSwapchain()
{
    RecreateSwapchain();
    InitSwapchainImages();
    viewport_.outDated = true;
    viewport_.DestroyViewportImages();
    viewport_.CreateViewportImages();
    imgui_.RecreateViewportDescriptorSets(viewport_);
}

void Engine::RecreateSwapchain()
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(Window::GetWindow(), &width, &height);
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
    UI::imDrawData = ImGui::GetDrawData();
}

Engine::~Engine()
{
    Device::GetBundle().device.destroyCommandPool(commandPool_);
    Device::GetBundle().device.waitIdle();
}
