#ifndef _SWAPCHAIN_H_
#define _SWAPCHAIN_H_

#include "../common.h"
#include "device.h"
#include "instance.h"
#include "pipeline.h"

class Swapchain
{
public:
    ~Swapchain();
    void CreateSwapchain(GLFWwindow* window);
    void QuerySwapchainSupportDetails();
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent(GLFWwindow* window);
    void CreateFrameBuffer();
    void DestroySwapchain();
};

struct SwapchainSupportDetails {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct SwapchainFrame {

    vk::Image image;
    vk::ImageView imageView;
    vk::Framebuffer framebuffer;
    vk::CommandBuffer commandBuffer;
    vk::Fence inFlight;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;
};

struct SwapchainDetails {

    vk::SwapchainKHR swapchain;
    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Extent2D extent;
};

inline SwapchainSupportDetails swapchainSupportDetails;
inline SwapchainDetails swapchainDetails;

#endif