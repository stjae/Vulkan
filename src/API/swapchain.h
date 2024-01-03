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

    std::vector<DescriptorSetLayoutData> descriptorSetLayoutData_;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    vk::DescriptorPoolCreateFlags descriptorPoolCreateFlags_;

    void QuerySwapchainSupport();
    void ChooseSurfaceFormat();
    void ChoosePresentMode();
    void ChooseExtent();

public:
    void CreateSwapchain();
    void CreateFrameBuffer(const vk::RenderPass& vkRenderPass);
    void PrepareFrames();
    void DestroySwapchain();
    ~Swapchain();

    void CreateDescriptorSetLayout();
    void RecordDrawCommand(GraphicsPipeline& pipeline, int frameIndex, const std::vector<std::shared_ptr<Mesh>>& meshes, uint32_t dynamicOffsetSize, ImDrawData* imDrawData);

    static const SwapchainBundle& Get() { return swapchainBundle_; }
    std::vector<SwapchainFrame> frames_;

    const std::vector<vk::DescriptorSetLayout>& GetDescriptorSetLayouts() { return descriptorSetLayouts_; }
};

#endif