#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "../common.h"
#include "swapchainBundle.h"
#include "device.h"
#include "instance.h"
#include "memory.h"
#include "pipeline.h"
#include "sync.h"

class Swapchain
{
    vk::SurfaceFormatKHR surfaceFormat_;
    vk::PresentModeKHR presentMode_;

    inline static SwapchainBundle swapchainBundle_;

    Pipeline pipeline_;
    vk::RenderPass renderPass_;
    std::vector<DescriptorSetLayoutData> descriptorSetLayoutData_;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;

    void QuerySwapchainSupport();
    void ChooseSurfaceFormat();
    void ChoosePresentMode();
    void ChooseExtent();

public:
    Swapchain();
    void CreateSwapchain();
    void CreateFrameBuffer();
    void PrepareFrames();
    void Destroy();
    ~Swapchain();

    void CreateRenderPass();
    void Draw(size_t frameIndex, ImDrawData* imDrawData);
    void Submit(size_t frameIndex);
    void Present(size_t frameIndex, const vk::ResultValue<unsigned int>& waitFrameImage);

    static const SwapchainBundle& Get() { return swapchainBundle_; }
    const vk::RenderPass& GetRenderPass() { return renderPass_; }
    std::vector<SwapchainFrame> frames;
};

#endif