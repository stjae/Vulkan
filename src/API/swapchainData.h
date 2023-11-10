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

    void CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
    {
        BufferInput input;
        input.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        input.size = sizeof(UBO);
        input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
        matrixUniformBuffer = std::make_shared<Buffer>(vkPhysicalDevice, vkDevice, input);

        input.size = sizeof(Light);
        lightUniformBuffer = std::make_shared<Buffer>(vkPhysicalDevice, vkDevice, input);

        matrixUniformBufferMemoryLocation = vkDevice.mapMemory(matrixUniformBuffer->vkDeviceMemory, 0, sizeof(UBO));
        lightUniformBufferMemoryLocation = vkDevice.mapMemory(lightUniformBuffer->vkDeviceMemory, 0, sizeof(Light));

        matrixUniformBufferDescriptorInfo.buffer = matrixUniformBuffer->vkBuffer;
        matrixUniformBufferDescriptorInfo.offset = 0;
        matrixUniformBufferDescriptorInfo.range = sizeof(UBO);

        lightUniformBufferDescriptorInfo.buffer = lightUniformBuffer->vkBuffer;
        lightUniformBufferDescriptorInfo.offset = 0;
        lightUniformBufferDescriptorInfo.range = sizeof(Light);
    }

    void WriteDescriptorSet(vk::Device vkDevice)
    {
        vk::WriteDescriptorSet matrixWriteInfo(descriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, matrixUniformBufferDescriptorInfo);
        vk::WriteDescriptorSet lightWriteInfo(descriptorSets[0], 1, 0, vk::DescriptorType::eUniformBuffer, nullptr, lightUniformBufferDescriptorInfo);

        vkDevice.updateDescriptorSets(matrixWriteInfo, nullptr);
        vkDevice.updateDescriptorSets(lightWriteInfo, nullptr);
    }
};

struct SwapchainDetail {

    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Extent2D extent;
};

#endif // __SWAPCHAINDATA_H__