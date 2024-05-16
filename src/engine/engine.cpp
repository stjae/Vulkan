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
    m_scene->ClearSubmitInfos();
    m_viewport.ClearSubmitInfos();

    DrawUI(currentImage.value);

    m_scene->Play();
    m_scene->Update();

    m_viewport.Draw(*m_scene, currentImage.value);
    m_swapchain.Draw(currentImage.value, UI::s_imDrawData);
    // TODO
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && m_viewport.m_isMouseHovered) {
        m_viewport.PickColor(Window::GetMousePosX(), Window::GetMousePosY(), 0);
    }
    m_imGui.AcceptDragDrop(m_viewport, *m_scene);

    std::vector<vk::SubmitInfo> submitInfos;
    submitInfos.insert(submitInfos.end(), m_scene->GetSubmitInfos().begin(), m_scene->GetSubmitInfos().end());
    submitInfos.insert(submitInfos.end(), m_viewport.GetSubmitInfos().begin(), m_viewport.GetSubmitInfos().end());
    submitInfos.push_back(m_swapchain.GetSubmitInfo());

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