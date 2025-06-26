#include "engine.h"

Engine::Engine()
{
    vkn::Command::CreateCommandPools();
    vkn::Command::AllocateCommandBuffers();
    vkn::Command::Begin(m_currentFrame);
    m_scene.Init(vkn::Command::GetCommandBuffer(m_currentFrame));
    m_swapchain.Init(vkn::Command::GetCommandBuffer(m_currentFrame));
    m_imGui.Init(vkn::Command::GetCommandBuffer(m_currentFrame), vkn::Swapchain::Get().renderPass);
    vkn::Command::End(m_currentFrame);
    vkn::Command::SubmitAndWait(m_currentFrame);
}

void Engine::Render()
{
    vkn::CHECK_RESULT(vkn::Device::Get().device.waitForFences(1, &vkn::Sync::GetInFlightFence(m_currentFrame), VK_TRUE, UINT64_MAX));
    vkn::CHECK_RESULT(vkn::Device::Get().device.resetFences(1, &vkn::Sync::GetInFlightFence(m_currentFrame)));

    auto currentImage = vkn::Device::Get().device.acquireNextImageKHR(vkn::Swapchain::Get().swapchain, UINT64_MAX, vkn::Sync::GetImageAvailableSemaphore(m_currentFrame), nullptr);

    if (currentImage.result == vk::Result::eErrorOutOfDateKHR || currentImage.result == vk::Result::eSuboptimalKHR || Window::s_resized) {
        Window::s_resized = false;
        UpdateSwapchain(vkn::Command::GetCommandBuffer(m_currentFrame));
        return;
    }

    vkn::Command::Begin(m_currentFrame);

    m_imGui.Draw(vkn::Command::GetCommandBuffer(m_currentFrame), m_scene, m_viewport, m_init);
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsUsing() && m_viewport.m_isMouseHovered && !m_scene.IsPlaying()) {
        m_viewport.PickColor(vkn::Command::GetCommandBuffer(m_currentFrame), Window::GetMousePosX(), Window::GetMousePosY(), m_scene);
    }
    m_imGui.AcceptDragDrop(m_scene);

    m_scene.Play();
    m_scene.Update(vkn::Command::GetCommandBuffer(m_currentFrame));

    m_swapchain.Draw(vkn::Command::GetCommandBuffer(m_currentFrame), currentImage.value, ImGui::GetDrawData());
    m_viewport.Draw(vkn::Command::GetCommandBuffer(m_currentFrame), m_scene);

    vkn::Command::End(m_currentFrame);

    vk::PipelineStageFlags waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::SubmitInfo submitInfo(1, &vkn::Sync::GetImageAvailableSemaphore(m_currentFrame), &waitStage, 1, &vkn::Command::GetCommandBuffer(m_currentFrame), 1, &vkn::Sync::GetRenderFinishedSemaphore(m_currentFrame));
    vkn::Device::Get().graphicsQueue.submit(submitInfo, vkn::Sync::GetInFlightFence(m_currentFrame));
    vk::PresentInfoKHR presentInfo(1, &vkn::Sync::GetRenderFinishedSemaphore(m_currentFrame), 1, &vkn::Swapchain::Get().swapchain, &currentImage.value);
    vkn::CHECK_RESULT(vkn::Device::Get().presentQueue.presentKHR(presentInfo));

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Engine::UpdateSwapchain(const vk::CommandBuffer& commandBuffer)
{
    RecreateSwapchain(commandBuffer);
    m_swapchain.InitSwapchainLayout(commandBuffer);
    m_viewport.m_outDated = true;
    m_viewport.DestroyImage();
    m_viewport.CreateImage();
    m_imGui.RecreateViewportDescriptorSet(m_viewport);
}

void Engine::RecreateSwapchain(const vk::CommandBuffer& commandBuffer)
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(Window::GetWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkn::Device::Get().device.waitIdle();

    m_swapchain.Destroy();
    m_swapchain.CreateSwapchain();
    m_swapchain.CreateFrameBuffer();
    m_swapchain.InitSwapchainLayout(commandBuffer);

    vkn::Sync::Destroy();
    vkn::Sync::Create();
}

Engine::~Engine()
{
    vkn::Device::Get().device.waitIdle();
    vkn::Command::DestroyCommandPools();
    vkn::Sync::Destroy();
}