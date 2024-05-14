#include "engine.h"

Engine::Engine()
{
    vkn::Sync::Create();
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

    m_scene->Play();
    m_scene->Update();

    DrawUI(currentImage.value);
    m_viewport.Draw(*m_scene, currentImage.value);
    m_swapchain.Draw(currentImage.value, UI::s_imDrawData);
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && m_viewport.m_isMouseHovered) {
        // TODO: imageindex
        // const int32_t* colorID = m_viewport.PickColor(Window::GetMousePosX(), Window::GetMousePosY());
        // m_scene->SelectByColorID(colorID[0], colorID[1]);
    }
    m_imGui.AcceptDragDrop(m_viewport, *m_scene);
    vk::SubmitInfo submitInfos[] = { m_scene->m_shadowMap.GetSubmitInfo(), m_viewport.GetSubmitInfo(), m_swapchain.GetSubmitInfo() };
    vkn::Device::Get().graphicsQueue.submit(submitInfos, vkn::Sync::GetInFlightFence());
    vk::PresentInfoKHR presentInfo(1, &vkn::Sync::GetRenderFinishedSemaphore(), 1, &vkn::Swapchain::Get().swapchain, &currentImage.value);
    vkn::CheckResult(vkn::Device::Get().presentQueue.presentKHR(presentInfo));

    vkn::Sync::SetNextFrameIndex();
}

void Engine::UpdateSwapchain()
{
    RecreateSwapchain();
    m_swapchain.InitSwapchain();
    m_viewport.m_outDated = true;
    m_viewport.DestroyImages();
    m_viewport.CreateImages();
    m_imGui.RecreateViewportDescriptorSets(m_viewport);
}

void Engine::RecreateSwapchain()
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
    m_swapchain.InitSwapchain();
    // recreate sync
}

void Engine::DrawUI(uint32_t imageIndex)
{
    m_imGui.Draw(*m_scene, m_viewport, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], imageIndex);
    UI::s_imDrawData = ImGui::GetDrawData();
}

Engine::~Engine()
{
    vkn::Device::Get().device.waitIdle();
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    vkn::Sync::Destroy();
}