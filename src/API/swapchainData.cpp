#include "swapchainData.h"

void SwapchainFrame::CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
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

void SwapchainFrame::WriteDescriptorSet(vk::Device vkDevice)
{
    vk::WriteDescriptorSet matrixWriteInfo(descriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, matrixUniformBufferDescriptorInfo);
    vk::WriteDescriptorSet lightWriteInfo(descriptorSets[0], 1, 0, vk::DescriptorType::eUniformBuffer, nullptr, lightUniformBufferDescriptorInfo);

    vkDevice.updateDescriptorSets(matrixWriteInfo, nullptr);
    vkDevice.updateDescriptorSets(lightWriteInfo, nullptr);
}