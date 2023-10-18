#ifndef _SWAPCHAIN_H_
#define _SWAPCHAIN_H_

#include "../common.h"
#include "config.h"
#include "device.h"
#include "instance.h"

class Swapchain
{
public:
    ~Swapchain();
    void CreateSwapchain(GLFWwindow* window);
    void QuerySwapchainSupportDetails();
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent(GLFWwindow* window);
    void DestroySwapchain();
};

#endif