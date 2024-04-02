#include "engine.h"

Engine::Engine()
{
    InitSwapchainImages();
    scene_ = std::make_unique<Scene>();

    vkn::Command::CreateCommandPool(commandPool_);
    vkn::Command::AllocateCommandBuffer(commandPool_, commandBuffer_);

    vkn::SetUpDescriptors();
    vkn::CreatePipeline();

    imgui_.Setup(swapchain_.GetRenderPass(), viewport_, *scene_);
}

void Engine::InitSwapchainImages()
{
    for (auto& frame : swapchain_.frames) {
        vkn::Command::Begin(frame.commandBuffer);
        vkn::Command::SetImageMemoryBarrier(frame.commandBuffer, frame.swapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, {}, {}, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe);
        frame.commandBuffer.end();
        vkn::Command::Submit(&frame.commandBuffer, 1);
    }
}

void Engine::Render()
{
    scene_->Update();

    int w, h;

    vkn::Device::GetBundle().device.waitForFences(1, &swapchain_.frames[frameIndex_].inFlight, VK_TRUE, UINT64_MAX);

    auto waitFrameImage = vkn::Device::GetBundle().device.acquireNextImageKHR(vkn::Swapchain::GetBundle().swapchain, UINT64_MAX, swapchain_.frames[frameIndex_].imageAvailable, nullptr);

    if (waitFrameImage.result == vk::Result::eErrorOutOfDateKHR || waitFrameImage.result == vk::Result::eSuboptimalKHR || Window::resized) {
        Window::resized = false;
        UpdateSwapchain();
        return;
    }

    vkn::Device::GetBundle().device.resetFences(1, &swapchain_.frames[frameIndex_].inFlight);

    swapchain_.Draw(frameIndex_, UI::imDrawData);
    viewport_.Draw(scene_->GetMeshes(), scene_->GetLightCount());
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && viewport_.isMouseHovered) {
        scene_->SelectByColorID(viewport_);
    }
    imgui_.AcceptDragDrop(viewport_, *scene_, frameIndex_);
    swapchain_.Submit(frameIndex_);
    swapchain_.Present(frameIndex_, waitFrameImage);

    frameIndex_ = (frameIndex_ + 1) % vkn::Swapchain::GetBundle().frameImageCount;
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

    vkn::Device::GetBundle().device.waitIdle();
    for (auto& frame : swapchain_.frames) {
        vkn::Device::GetBundle().device.destroyCommandPool(frame.commandPool);
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
    vkn::Device::GetBundle().device.destroyCommandPool(commandPool_);
    vkn::Device::GetBundle().device.waitIdle();
}
