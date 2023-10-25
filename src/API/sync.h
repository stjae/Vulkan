#ifndef _SYNC_H_
#define _SYNC_H_

#include "../common.h"
#include "device.h"

inline vk::Semaphore MakeSemaphore(vk::Device vkDevice)
{
    vk::SemaphoreCreateInfo semaphoreInfo;

    return vkDevice.createSemaphore(semaphoreInfo);
}

inline vk::Fence MakeFence(vk::Device vkDevice)
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    return vkDevice.createFence(fenceInfo);
}

#endif