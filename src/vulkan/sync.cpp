#include "sync.h"

namespace vkn {
Sync::Sync()
{
    Create();
}
void Sync::Create()
{
    s_inFlightFences.resize(MAX_FRAME);
    s_commandFence = CreateFence();
    s_imageAvailableSemaphores.resize(MAX_FRAME);
    s_renderFinishedSemaphores.resize(MAX_FRAME);
    s_shadowCubemapSemaphores.resize(MAX_FRAME);
    for (int i = 0; i < MAX_FRAME; i++) {
        s_inFlightFences[i] = CreateFence();
        s_imageAvailableSemaphores[i] = CreateSemaphore();
        s_renderFinishedSemaphores[i] = CreateSemaphore();
        s_shadowCubemapSemaphores[i] = CreateSemaphore();
    }
}
void Sync::Destroy()
{
    Device::Get().device.destroy(s_commandFence);
    for (int i = 0; i < MAX_FRAME; i++) {
        Device::Get().device.destroy(s_inFlightFences[i]);
        Device::Get().device.destroy(s_imageAvailableSemaphores[i]);
        Device::Get().device.destroy(s_renderFinishedSemaphores[i]);
        Device::Get().device.destroy(s_shadowCubemapSemaphores[i]);
    }
}
vk::Semaphore Sync::CreateSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;
    return Device::Get().device.createSemaphore(semaphoreInfo);
}
vk::Fence Sync::CreateFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    return Device::Get().device.createFence(fenceInfo);
}
}; // namespace vkn