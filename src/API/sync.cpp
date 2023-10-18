#include "sync.h"

vk::Semaphore Sync::MakeSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;

    return device.createSemaphore(semaphoreInfo);
}

vk::Fence Sync::MakeFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    return device.createFence(fenceInfo);
}