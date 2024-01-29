#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "../common.h"
#include "device/device.h"
#include "device/instance.h"
#include "command.h"
#include "memory.h"
#include "pipeline.h"
#include "sync.h"

struct SwapchainFrame
{
    vk::Framebuffer framebuffer;

    vk::Image swapchainImage;
    vk::ImageView swapchainImageView;

    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;
    vk::CommandBuffer renderPassCommandBuffer;

    vk::Fence inFlight;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;
};

struct SwapchainBundle
{
    vk::Extent2D swapchainImageExtent;
    vk::SwapchainKHR swapchain;
};

class Swapchain
{
    std::vector<vk::SurfaceFormatKHR> supportedFormats_;
    std::vector<vk::PresentModeKHR> supportedPresentModes_;

    vk::SurfaceFormatKHR surfaceFormat_;
    vk::PresentModeKHR presentMode_;

    inline static SwapchainBundle swapchainBundle_;
    inline static uint32_t frameImageCount_;

    Pipeline pipeline_;
    vk::RenderPass renderPass_;
    std::vector<DescriptorSetLayoutData> descriptorSetLayoutData_;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;

    void QuerySwapchainSupport();
    void ChooseSurfaceFormat();
    void ChoosePresentMode();
    void ChooseExtent();
    void WriteSwapchainCreateInfo();

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

    static uint32_t GetFrameImageCount() { return frameImageCount_; }
    static const SwapchainBundle& GetBundle() { return swapchainBundle_; }
    const vk::RenderPass& GetRenderPass() { return renderPass_; }

    inline static vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<SwapchainFrame> frames;
};

#endif