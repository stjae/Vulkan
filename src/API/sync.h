#ifndef _SYNC_H_
#define _SYNC_H_

#include "../common.h"
#include "device.h"

static vk::Semaphore MakeSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;

    return Device::GetHandle().device.createSemaphore(semaphoreInfo);
}

static vk::Fence MakeFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    return Device::GetHandle().device.createFence(fenceInfo);
}

#endif