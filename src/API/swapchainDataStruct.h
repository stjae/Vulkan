#ifndef _SWAPCHAINDATASTRUCT_H_
#define _SWAPCHAINDATASTRUCT_H_

#include "../common.h"
#include "buffer.h"

struct SwapchainSupportDetail {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct UBO {

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct SwapchainFrame {

    vk::Image image;
    vk::ImageView imageView;
    vk::Framebuffer framebuffer;

    vk::CommandBuffer commandBuffer;

    vk::Fence inFlight;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;

    UBO uboData;
    std::shared_ptr<Buffer> uboDataBuffer;
    void* uboDataMemoryLocation;

    vk::DescriptorBufferInfo uniformBufferDescriptorInfo;
    std::vector<vk::DescriptorSet> descriptorSets;

    void CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
    {
        BufferInput input;
        input.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        input.size = sizeof(UBO);
        input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
        uboDataBuffer = std::make_shared<Buffer>(vkPhysicalDevice, vkDevice, input);

        uboDataMemoryLocation = vkDevice.mapMemory(uboDataBuffer->vkDeviceMemory, 0, sizeof(UBO));

        uniformBufferDescriptorInfo.buffer = uboDataBuffer->vkBuffer;
        uniformBufferDescriptorInfo.offset = 0;
        uniformBufferDescriptorInfo.range = sizeof(UBO);
    }

    void WriteDescriptorSet(vk::Device vkDevice)
    {
        vk::WriteDescriptorSet writeInfo(descriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, uniformBufferDescriptorInfo);

        vkDevice.updateDescriptorSets(writeInfo, nullptr);
    }
};

struct SwapchainDetail {

    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Extent2D extent;
};

#endif