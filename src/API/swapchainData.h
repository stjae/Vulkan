#ifndef __SWAPCHAINDATA_H__
#define __SWAPCHAINDATA_H__

#include "../common.h"
#include "buffer.h"
#include "../camera.h"
#include "../light.h"

struct UBO {

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

    glm::vec3 eye;
};

struct SwapchainSupportDetail {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct SwapchainFrame {

    vk::Image image;
    vk::ImageView imageView;

    vk::Image depthImage;
    vk::ImageView depthImageView;

    vk::Framebuffer framebuffer;

    vk::CommandBuffer commandBuffer;

    vk::Fence inFlight;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;

    std::shared_ptr<Buffer> matrixUniformBuffer;
    void* matrixUniformBufferMemoryLocation;

    std::shared_ptr<Buffer> lightUniformBuffer;
    void* lightUniformBufferMemoryLocation;

    vk::DescriptorBufferInfo matrixUniformBufferDescriptorInfo;
    vk::DescriptorBufferInfo lightUniformBufferDescriptorInfo;

    std::vector<vk::DescriptorSet> descriptorSets;

    void CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice);
    void WriteDescriptorSet(vk::Device vkDevice);

    vk::Image CreateImage();
    vk::ImageView CreateImageView();
};

struct SwapchainDetail {

    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Extent2D extent;
};

#endif // __SWAPCHAINDATA_H__