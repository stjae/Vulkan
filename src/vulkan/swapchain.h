// Swapchain creation, drawing, and destruction
// スワップチェーンの作成・描画・破棄

#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "../common.h"
#include "device.h"
#include "instance.h"
#include "command.h"
#include "descriptor.h"
#include "memory.h"
#include "sync.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"

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

    void QuerySwapchainSupport();
    void ChooseSurfaceFormat();
    void ChoosePresentMode();
    void ChooseExtent();

public:
    ~Swapchain();
    void Init(const vk::CommandBuffer& commandBuffer);
    void CreateSwapchain();
    void InitSwapchainLayout(const vk::CommandBuffer& commandBuffer);
    void CreateRenderPass();
    void CreateFrameBuffer();
    void Draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex, ImDrawData* imDrawData);
    void Destroy();

    static const Bundle& Get() { return s_bundle; }
};
} // namespace vkn

#endif