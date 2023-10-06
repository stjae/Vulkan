#ifndef _VKSTRUCT_H_
#define _VKSTRUCT_H_

#include "../common.h"

struct QueueFamilyIndices {

    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    QueueFamilyIndices() {}
    QueueFamilyIndices(const QueueFamilyIndices&) = delete;
    QueueFamilyIndices& operator=(const QueueFamilyIndices&) = delete;
};

struct SwapchainSupportDetails {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    SwapchainSupportDetails() {}
    SwapchainSupportDetails(const SwapchainSupportDetails&) = delete;
    SwapchainSupportDetails& operator=(const SwapchainSupportDetails&) = delete;
};

struct SwapchainDetails {

    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    vk::Format format;
    vk::Extent2D extent;

    SwapchainDetails() {}
    SwapchainDetails(const SwapchainDetails&) = delete;
    SwapchainDetails& operator=(const SwapchainDetails&) = delete;
};

#endif