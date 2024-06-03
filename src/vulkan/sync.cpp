#include "sync.h"

namespace vkn {
Sync::Sync()
{
    Create();
}
void Sync::Create()
{
    s_inFlightFences.resize(MAX_FRAME);
    s_commandFence = CreateVkFence();
    s_imageAvailableSemaphores.resize(MAX_FRAME);
    s_renderFinishedSemaphores.resize(MAX_FRAME);
    s_shadowMapSemaphores.resize(MAX_FRAME);
    s_viewportSemaphores.resize(MAX_FRAME);
    for (int i = 0; i < MAX_FRAME; i++) {
        s_inFlightFences[i] = CreateVkFence();
        s_imageAvailableSemaphores[i] = CreateVkSemaphore();
        s_renderFinishedSemaphores[i] = CreateVkSemaphore();
        s_shadowMapSemaphores[i] = CreateVkSemaphore();
        s_viewportSemaphores[i] = CreateVkSemaphore();
    }
}
void Sync::Destroy()
{
    Device::Get().device.destroy(s_commandFence);
    for (int i = 0; i < MAX_FRAME; i++) {
        Device::Get().device.destroy(s_inFlightFences[i]);
        Device::Get().device.destroy(s_imageAvailableSemaphores[i]);
        Device::Get().device.destroy(s_renderFinishedSemaphores[i]);
        Device::Get().device.destroy(s_shadowMapSemaphores[i]);
        Device::Get().device.destroy(s_viewportSemaphores[i]);
    }
}
vk::Semaphore Sync::CreateVkSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;
    return Device::Get().device.createSemaphore(semaphoreInfo);
}
vk::Fence Sync::CreateVkFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    return Device::Get().device.createFence(fenceInfo);
}
}; // namespace vkn