#ifndef SWAPCHAINDATA_H
#define SWAPCHAINDATA_H

#include "../common.h"
#include "commands.h"
#include "descriptor.h"
#include "buffer.h"
#include "image.h"
#include "../camera.h"
#include "../mesh.h"

struct SwapchainSupport
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct SwapchainFrame
{
    vk::Framebuffer framebuffer;

    vk::Image swapchainImage;
    vk::ImageView swapchainImageView;
    //    Image depthImage;

    Command command;
    Descriptor descriptor;

    vk::Fence inFlight;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;
};

struct SwapchainBundle
{
    SwapchainSupport support;
    vk::Extent2D swapchainImageExtent;
    vk::SwapchainKHR swapchain;
    size_t frameCount;
};

#endif