#ifndef _SYNC_H_
#define _SYNC_H_

#include "../common.h"
#include "device.h"

inline vk::Semaphore MakeSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;

    return Device::GetDevice().createSemaphore(semaphoreInfo);
}

inline vk::Fence MakeFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    return Device::GetDevice().createFence(fenceInfo);
}

#endif