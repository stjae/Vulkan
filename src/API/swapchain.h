#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "../common.h"
#include "swapchainData.h"
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
    vk::DescriptorPoolCreateFlags descriptorPoolCreateFlags_;

    void QuerySwapchainSupport();
    void ChooseSurfaceFormat();
    void ChoosePresentMode();
    void ChooseExtent();

public:
    Swapchain();
    void CreateSwapchain();
    void CreateFrameBuffer();
    void PrepareFrames();
    void DestroySwapchain();
    ~Swapchain();

    void CreateRenderPass();
    void Draw(size_t frameIndex, const std::vector<Mesh>& meshes, uint32_t dynamicOffsetSize, ImDrawData* imDrawData);

    static const SwapchainBundle& Get() { return swapchainBundle_; }
    const vk::RenderPass& GetRenderPass() { return renderPass_; }
    std::vector<SwapchainFrame> frames_;
};

#endif