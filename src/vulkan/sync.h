#ifndef SYNC_H
#define SYNC_H

#include "../common.h"
#include "device.h"

namespace vkn {
class Sync
{
    inline static vk::Fence s_inFlightFence;
    inline static vk::Fence s_commandFence;
    inline static vk::Semaphore s_imageAvailableSemaphore;
    inline static vk::Semaphore s_renderFinishedSemaphore;

public:
    Sync();
    static void Create();
    static void Destroy();
    static vk::Semaphore CreateVkSemaphore();
    static vk::Fence CreateVkFence();
    static const vk::Fence& GetInFlightFence() { return s_inFlightFence; }
    static const vk::Fence& GetCommandFence() { return s_commandFence; }
    static const vk::Semaphore& GetImageAvailableSemaphore() { return s_imageAvailableSemaphore; }
    static const vk::Semaphore& GetRenderFinishedSemaphore() { return s_renderFinishedSemaphore; }
};
} // namespace vkn

#endif