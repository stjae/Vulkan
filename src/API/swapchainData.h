#ifndef __SWAPCHAINDATA_H__
#define __SWAPCHAINDATA_H__

#include "../common.h"
#include "buffer.h"
#include "../camera.h"
#include "../image.h"
#include "../mesh.h"

struct SwapchainSupportDetail {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct SwapchainFrame {

    vk::Image swapchainVkImage;
    vk::ImageView swapchainVkImageView;

    Image depthImage;

    vk::Framebuffer framebuffer;
    vk::CommandBuffer commandBuffer;

    vk::Fence inFlight;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;

    std::vector<vk::DescriptorSet> descriptorSets;

    const vk::PhysicalDevice& vkPhysicalDevice;
    const vk::Device& vkDevice;

    SwapchainFrame(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice) : vkPhysicalDevice(vkPhysicalDevice), vkDevice(vkDevice), depthImage(vkPhysicalDevice, vkDevice) {}
};

struct SwapchainDetail {

    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Extent2D extent;

    const vk::PhysicalDevice& vkPhysicalDevice;
    const vk::Device& vkDevice;

    SwapchainDetail(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice) : vkPhysicalDevice(vkPhysicalDevice), vkDevice(vkDevice) {}
};

#endif // __SWAPCHAINDATA_H__