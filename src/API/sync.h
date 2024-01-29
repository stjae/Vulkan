#ifndef SYNC_H
#define SYNC_H

#include "../common.h"
#include "device/device.h"

static vk::Semaphore MakeSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;

    return Device::GetBundle().device.createSemaphore(semaphoreInfo);
}

static vk::Fence MakeFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    return Device::GetBundle().device.createFence(fenceInfo);
}

#endif