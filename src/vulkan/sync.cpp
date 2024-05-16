#include "sync.h"

namespace vkn {
void Sync::Create()
{
    s_inFlightFences.resize(MAX_FRAME);
    s_imageAvailableSemaphores.resize(MAX_FRAME);
    s_renderFinishedSemaphores.resize(MAX_FRAME);
    s_shadowMapSemaphores.resize(MAX_FRAME);
    s_sceneSemaphores.resize(MAX_FRAME);
    s_viewportSemaphores.resize(MAX_FRAME);

    for (int i = 0; i < MAX_FRAME; i++) {
        s_inFlightFences[i] = CreateFence();
        s_imageAvailableSemaphores[i] = CreateSemaphore();
        s_renderFinishedSemaphores[i] = CreateSemaphore();
        s_shadowMapSemaphores[i] = CreateSemaphore();
        s_sceneSemaphores[i] = CreateSemaphore();
        s_viewportSemaphores[i] = CreateSemaphore();
    }
}
void Sync::Destroy()
{
    for (int i = 0; i < MAX_FRAME; i++) {
        Device::Get().device.destroy(s_inFlightFences[i]);
        Device::Get().device.destroy(s_imageAvailableSemaphores[i]);
        Device::Get().device.destroy(s_renderFinishedSemaphores[i]);
        Device::Get().device.destroy(s_shadowMapSemaphores[i]);
        Device::Get().device.destroy(s_sceneSemaphores[i]);
        Device::Get().device.destroy(s_viewportSemaphores[i]);
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