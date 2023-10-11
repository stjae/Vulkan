#include "sync.h"

vk::Semaphore Sync::CreateVkSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;

    return device.createSemaphore(semaphoreInfo);
}

vk::Fence Sync::CreateVkFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

    return device.createFence(fenceInfo);
}

Sync::~Sync()
{
    device.destroyFence(inFlightFence);
    device.destroySemaphore(imageAvailable);
    device.destroySemaphore(renderFinished);
}