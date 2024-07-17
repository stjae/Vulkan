#include "engine.h"

Engine::Engine() : m_init(true)
{
    m_scene.Init();
    m_imGui.Init(vkn::Swapchain::Get().renderPass);
    m_commandBuffers = { m_scene.m_commandBuffer, m_viewport.m_commandBuffer, m_swapchain.m_commandBuffer };
}

void Engine::Render()
{
    vkn::CheckResult(vkn::Device::Get().device.waitForFences(1, &vkn::Sync::GetInFlightFence(), VK_TRUE, UINT64_MAX));
    vkn::CheckResult(vkn::Device::Get().device.resetFences(1, &vkn::Sync::GetInFlightFence()));

    auto currentImage = vkn::Device::Get().device.acquireNextImageKHR(vkn::Swapchain::Get().swapchain, UINT64_MAX, vkn::Sync::GetImageAvailableSemaphore(), nullptr);

    if (currentImage.result == vk::Result::eErrorOutOfDateKHR || currentImage.result == vk::Result::eSuboptimalKHR || Window::s_resized) {
        Window::s_resized = false;
        UpdateSwapchain();
        return;
    }

    for (auto& cb : m_commandBuffers)
        vkn::Command::Begin(cb);

    m_imGui.Draw(m_scene, m_viewport, m_init);
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsUsing() && m_viewport.m_isMouseHovered && !m_scene.IsPlaying()) {
        m_viewport.PickColor(Window::GetMousePosX(), Window::GetMousePosY(), m_scene);
    }
    m_imGui.AcceptDragDrop(m_scene);

    m_scene.Play();
    m_scene.Update();

    m_swapchain.Draw(currentImage.value, ImGui::GetDrawData());
    m_viewport.Draw(m_scene);

    for (auto& cb : m_commandBuffers)
        vkn::Command::End(cb);

    vk::PipelineStageFlags waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::SubmitInfo submitInfo(1, &vkn::Sync::GetImageAvailableSemaphore(), &waitStage, m_commandBuffers.size(), m_commandBuffers.data(), 1, &vkn::Sync::GetRenderFinishedSemaphore());
    vkn::Device::Get().graphicsQueue.submit(submitInfo, vkn::Sync::GetInFlightFence());
    vk::PresentInfoKHR presentInfo(1, &vkn::Sync::GetRenderFinishedSemaphore(), 1, &vkn::Swapchain::Get().swapchain, &currentImage.value);
    vkn::CheckResult(vkn::Device::Get().presentQueue.presentKHR(presentInfo));
}

void Engine::UpdateSwapchain()
{
    RecreateSwapchain();
    m_swapchain.InitSwapchain();
    m_viewport.m_outDated = true;
    m_viewport.DestroyImage();
    m_viewport.CreateImage();
    m_imGui.RecreateViewportDescriptorSet(m_viewport);
}

void Engine::RecreateSwapchain()
{
    // TODO: increase speed
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
    m_swapchain.InitSwapchain();

    vkn::Sync::Destroy();
    vkn::Sync::Create();
}

Engine::~Engine()
{
    vkn::Device::Get().device.waitIdle();
    vkn::Sync::Destroy();
}