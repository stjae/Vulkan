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
public:
    Swapchain(const Device& device, const vk::RenderPass& vkRenderPass)
        : vkDevice(device.vkDevice), vkPhysicalDevice(device.vkPhysicalDevice), vkSurface(device.instance.vkSurface), vkRenderPass(vkRenderPass), detail(device.vkPhysicalDevice, device.vkDevice) {}
    void CreateSwapchain(GLFWwindow* window, const Device& device);
    void QueryswapchainSupportDetails(const Device& device);
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent(GLFWwindow* window);
    void CreateFrameBuffer();
    void PrepareFrames();
    void DestroySwapchain();
    ~Swapchain();

    vk::SwapchainKHR vkSwapchain;
    SwapchainSupportDetail supportDetail;
    SwapchainDetail detail;

private:
    const vk::Device& vkDevice;
    const vk::PhysicalDevice& vkPhysicalDevice;
    const VkSurfaceKHR& vkSurface;
    const vk::RenderPass& vkRenderPass;
};

#endif