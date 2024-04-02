#ifndef SYNC_H
#define SYNC_H

#include "../common.h"
#include "device.h"

namespace vkn {
static vk::Semaphore CreateSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;

    return Device::GetBundle().device.createSemaphore(semaphoreInfo);
}
static vk::Fence CreateFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    return Device::GetBundle().device.createFence(fenceInfo);
}
} // namespace vkn

#endif