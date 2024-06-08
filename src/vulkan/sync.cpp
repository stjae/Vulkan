#include "sync.h"

namespace vkn {
Sync::Sync()
{
    Create();
}
void Sync::Create()
{
    s_commandFence = CreateVkFence();
    s_inFlightFence = CreateVkFence();
    s_imageAvailableSemaphore = CreateVkSemaphore();
    s_renderFinishedSemaphore = CreateVkSemaphore();
}
void Sync::Destroy()
{
    Device::Get().device.destroy(s_commandFence);
    Device::Get().device.destroy(s_inFlightFence);
    Device::Get().device.destroy(s_imageAvailableSemaphore);
    Device::Get().device.destroy(s_renderFinishedSemaphore);
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