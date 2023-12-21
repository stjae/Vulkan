#ifndef _SWAPCHAIN_H_
#define _SWAPCHAIN_H_

#include "../common.h"
#include "swapchainData.h"
#include "device.h"
#include "instance.h"
#include "memory.h"
#include "pipeline.h"
#include "sync.h"

class Swapchain
{
    void QuerySwapchainSupportDetail();
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent();

    inline static vk::SwapchainKHR handle_;
    inline static SwapchainSupportDetail supportDetail_;
    inline static SwapchainDetail detail_;

public:
    void CreateSwapchain();
    void CreateFrameBuffer(const vk::RenderPass& vkRenderPass);
    void PrepareFrames();
    void DestroySwapchain();
    ~Swapchain();

    static const vk::SwapchainKHR& GetHandle() { return handle_; }
    static const SwapchainSupportDetail& GetSupportDetail() { return supportDetail_; }
    static SwapchainDetail& GetDetail() { return detail_; }
};

#endif