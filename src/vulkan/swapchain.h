#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "../common.h"
#include "device.h"
#include "instance.h"
#include "command.h"
#include "descriptor.h"
#include "memory.h"
#include "sync.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_vulkan.h"

namespace vkn {
class Swapchain
{
    friend class Engine;
    struct Image
    {
        vk::Framebuffer framebuffer;
        vk::Image image;
        vk::ImageView imageView;
    };
    struct Bundle
    {
        uint32_t frameImageCount;
        vk::SurfaceFormatKHR surfaceFormat;
        vk::PresentModeKHR presentMode;
        vk::Extent2D swapchainImageExtent;
        vk::SwapchainKHR swapchain;
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        vk::RenderPass renderPass;
    } inline static s_bundle;
    std::vector<vk::SurfaceFormatKHR> m_supportedFormats;
    std::vector<vk::PresentModeKHR> m_supportedPresentModes;
    std::vector<Image> m_swapchainImages;

    vk::CommandPool m_commandPool;

    void QuerySwapchainSupport();
    void ChooseSurfaceFormat();
    void ChoosePresentMode();
    void ChooseExtent();

public:
    Swapchain();
    ~Swapchain();
    void CreateSwapchain();
    void InitSwapchain();
    void CreateRenderPass();
    void CreateFrameBuffer();
    void Draw(uint32_t imageIndex, ImDrawData* imDrawData);
    void Destroy();

    static const Bundle& Get() { return s_bundle; }
    vk::CommandBuffer m_commandBuffer;
};
} // namespace vkn

#endif