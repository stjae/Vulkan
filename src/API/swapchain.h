#ifndef _SWAPCHAIN_H_
#define _SWAPCHAIN_H_

#include "../common.h"
#include "device.h"

struct SwapchainSupportDetails;
struct SwapchainDetails;

class Swapchain
{
public:
    ~Swapchain();
    void QuerySwapchainSupportDetails();
    void CreateSwapchain();
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent();

    static SwapchainSupportDetails swapchainSupportDetails;
    static SwapchainDetails swapchainDetails;
};

struct SwapchainSupportDetails {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    SwapchainSupportDetails() {}
    SwapchainSupportDetails(const SwapchainSupportDetails&) = delete;
    SwapchainSupportDetails& operator=(const SwapchainSupportDetails&) = delete;
};

struct SwapchainFrame {

    vk::Image image;
    vk::ImageView imageView;
};

struct SwapchainDetails {

    vk::SwapchainKHR swapchain;
    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Extent2D extent;

    SwapchainDetails() {}
    SwapchainDetails(const SwapchainDetails&) = delete;
    SwapchainDetails& operator=(const SwapchainDetails&) = delete;
};

#endif