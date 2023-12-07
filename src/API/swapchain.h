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
    void QueryswapchainSupportDetails();
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent();

public:
    void CreateSwapchain();
    void CreateFrameBuffer(const vk::RenderPass& vkRenderPass);
    void PrepareFrames();
    void DestroySwapchain();
    static vk::SwapchainKHR& GetSwapchain()
    {
        static vk::SwapchainKHR swapchain;
        return swapchain;
    }
    static SwapchainDetail& GetDetail()
    {
        static SwapchainDetail detail;
        return detail;
    }
    static SwapchainSupportDetail& GetSupportDetail()
    {
        static SwapchainSupportDetail supportDetail;
        return supportDetail;
    }
    ~Swapchain();
};

#endif