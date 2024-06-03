#ifndef SYNC_H
#define SYNC_H

#include "../common.h"
#include "device.h"

namespace vkn {
class Sync
{
    inline static int s_currentFrameIndex = 0;
    inline static std::vector<vk::Fence> s_inFlightFences;
    inline static vk::Fence s_commandFence;
    inline static std::vector<vk::Semaphore> s_imageAvailableSemaphores;
    inline static std::vector<vk::Semaphore> s_renderFinishedSemaphores;
    inline static std::vector<vk::Semaphore> s_shadowMapSemaphores;
    inline static std::vector<vk::Semaphore> s_viewportSemaphores;

public:
    static void SetNextFrameIndex() { s_currentFrameIndex = (s_currentFrameIndex + 1) % MAX_FRAME; }

    Sync();
    static void Create();
    static void Destroy();
    static vk::Semaphore CreateVkSemaphore();
    static vk::Fence CreateVkFence();
    static int GetCurrentFrameIndex() { return s_currentFrameIndex; }
    static const vk::Fence& GetInFlightFence() { return s_inFlightFences[s_currentFrameIndex]; }
    static const vk::Fence& GetCommandFence() { return s_commandFence; }
    static const vk::Semaphore& GetImageAvailableSemaphore() { return s_imageAvailableSemaphores[s_currentFrameIndex]; }
    static const vk::Semaphore& GetRenderFinishedSemaphore() { return s_renderFinishedSemaphores[s_currentFrameIndex]; }
    static const vk::Semaphore& GetShadowMapSemaphore() { return s_shadowMapSemaphores[s_currentFrameIndex]; }
    static const vk::Semaphore& GetViewportSemaphore() { return s_viewportSemaphores[s_currentFrameIndex]; }
};
} // namespace vkn

#endif