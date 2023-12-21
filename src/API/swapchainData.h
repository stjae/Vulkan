#ifndef __SWAPCHAINDATA_H__
#define __SWAPCHAINDATA_H__

#include "../common.h"
#include "buffer.h"
#include "image.h"
#include "../camera.h"
#include "../mesh.h"

struct SwapchainSupportDetail
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct SwapchainFrame
{
    vk::Image swapchainVkImage;
    vk::ImageView swapchainVkImageView;

    Image depthImage;

    vk::Framebuffer framebuffer;
    vk::CommandBuffer commandBuffer;

    vk::Fence inFlight;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;

    std::vector<vk::DescriptorSet> descriptorSets;
};

struct SwapchainDetail
{
    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Format depthImageFormat;
    vk::Extent2D extent;
};

#endif // __SWAPCHAINDATA_H__