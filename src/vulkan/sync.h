#ifndef SYNC_H
#define SYNC_H

#include "../common.h"
#include "device.h"

namespace vkn {
class Sync
{
    inline static vk::Fence s_inFlightFences[MAX_FRAMES_IN_FLIGHT];
    inline static vk::Semaphore s_imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    inline static vk::Semaphore s_renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    inline static vk::Fence s_commandFence;

public:
    Sync();
    static void Create();
    static void Destroy();
    static void CreateSemaphores();
    static void CreateFences();
    static const vk::Fence& GetInFlightFence(int index) { return s_inFlightFences[index]; }
    static const vk::Semaphore& GetImageAvailableSemaphore(int index) { return s_imageAvailableSemaphores[index]; }
    static const vk::Semaphore& GetRenderFinishedSemaphore(int index) { return s_renderFinishedSemaphores[index]; }
    static const vk::Fence& GetCommandFence() { return s_commandFence; }
};
} // namespace vkn

#endif