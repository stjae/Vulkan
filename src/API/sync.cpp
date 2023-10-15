#include "sync.h"

vk::Semaphore Sync::MakeSemaphore()
{
    vk::SemaphoreCreateInfo semaphoreInfo;

    return device.createSemaphore(semaphoreInfo);
}

vk::Fence Sync::MakeFence()
{
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

    return device.createFence(fenceInfo);
}

Sync::~Sync()
{
    for (auto& frame : swapchainDetails.frames) {
        device.destroyFence(frame.inFlight);
        device.destroySemaphore(frame.imageAvailable);
        device.destroySemaphore(frame.renderFinished);
    }
}