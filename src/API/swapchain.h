#ifndef _SWAPCHAIN_H_
#define _SWAPCHAIN_H_

#include "../common.h"
#include "swapchainDataStruct.h"
#include "device.h"
#include "instance.h"
#include "memory.h"
#include "pipeline.h"
#include "sync.h"

class Swapchain
{
public:
    Swapchain(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const VkSurfaceKHR& vkSurface, const vk::RenderPass& vkRenderPass)
        : vkPhysicalDevice(vkPhysicalDevice), vkDevice(vkDevice), vkSurface(vkSurface), vkRenderPass(vkRenderPass) {}
    void CreateSwapchain(GLFWwindow* window, Device& device);
    void QueryswapchainSupportDetails(Device& device);
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent(GLFWwindow* window);
    void CreateFrameBuffer();
    void PrepareFrames();
    void DestroySwapchain();
    ~Swapchain();

    SwapchainSupportDetail supportDetail;
    SwapchainDetail detail;

private:
    const vk::PhysicalDevice& vkPhysicalDevice;
    const vk::Device& vkDevice;
    const VkSurfaceKHR& vkSurface;
    const vk::RenderPass& vkRenderPass;
};

#endif