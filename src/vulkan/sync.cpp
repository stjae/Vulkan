#include "sync.h"

namespace vkn {
Sync::Sync()
{
    Create();
}
void Sync::Create()
{
    CreateFences();
    CreateSemaphores();
}
void Sync::Destroy()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        Device::Get().device.destroy(s_inFlightFences[i]);
        Device::Get().device.destroy(s_imageAvailableSemaphores[i]);
        Device::Get().device.destroy(s_renderFinishedSemaphores[i]);
    }
    Device::Get().device.destroy(s_commandFence);
}
void Sync::CreateSemaphores()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::SemaphoreCreateInfo semaphoreInfo;
        s_imageAvailableSemaphores[i] = Device::Get().device.createSemaphore(semaphoreInfo);
        s_renderFinishedSemaphores[i] = Device::Get().device.createSemaphore(semaphoreInfo);
    }
}
void Sync::CreateFences()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::FenceCreateInfo fenceInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        s_inFlightFences[i] = Device::Get().device.createFence(fenceInfo);
    }

    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    s_commandFence = Device::Get().device.createFence(fenceInfo);
}
}; // namespace vkn