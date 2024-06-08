#include "engine.h"

Engine::Engine() : m_init(true)
{
    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffers);
    m_scene = std::make_unique<Scene>();
    m_imGui.Setup(vkn::Swapchain::Get().renderPass, m_viewport, *m_scene);
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

    vkn::Device::s_submitInfos.clear();

    m_imGui.Draw(*m_scene, m_viewport, m_init);
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && m_viewport.m_isMouseHovered && !m_scene->IsPlaying()) {
        m_viewport.PickColor(Window::GetMousePosX(), Window::GetMousePosY(), *m_scene);
    }
    m_imGui.AcceptDragDrop(m_viewport, *m_scene);

    m_scene->Play();
    m_scene->Update();

    m_swapchain.Draw(currentImage.value, ImGui::GetDrawData());
    m_viewport.Draw(*m_scene);

    vkn::Device::Get().graphicsQueue.submit(vkn::Device::s_submitInfos, vkn::Sync::GetInFlightFence());
    vk::PresentInfoKHR presentInfo(1, &vkn::Sync::GetRenderFinishedSemaphore(), 1, &vkn::Swapchain::Get().swapchain, &currentImage.value);
    vkn::CheckResult(vkn::Device::Get().presentQueue.presentKHR(presentInfo));

    vkn::Sync::SetNextFrameIndex();
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
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    vkn::Sync::Destroy();
}