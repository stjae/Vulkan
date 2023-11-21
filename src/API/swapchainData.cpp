#include "swapchainData.h"

void SwapchainFrame::CreateUniformBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    vk::PhysicalDeviceProperties props = vkPhysicalDevice.getProperties();

    BufferInput input;
    input.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    input.size = sizeof(UBO);
    input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
    matrixUniformBuffers.emplace_back(std::make_shared<Buffer>(vkPhysicalDevice, vkDevice));
    matrixUniformBuffers.back()->CreateBuffer(input);
    matrixUniformBuffers.back()->memory.memoryLocation = vkDevice.mapMemory(matrixUniformBuffers.back()->memory.vkDeviceMemory, 0, sizeof(UBO));
    matrixUniformBuffers.emplace_back(std::make_shared<Buffer>(vkPhysicalDevice, vkDevice));
    matrixUniformBuffers.back()->CreateBuffer(input);
    matrixUniformBuffers.back()->memory.memoryLocation = vkDevice.mapMemory(matrixUniformBuffers.back()->memory.vkDeviceMemory, 0, sizeof(UBO));

    input.size = sizeof(Light);
    lightUniformBuffer = std::make_shared<Buffer>(vkPhysicalDevice, vkDevice);
    lightUniformBuffer->CreateBuffer(input);
    lightUniformBuffer->memory.memoryLocation = vkDevice.mapMemory(lightUniformBuffer->memory.vkDeviceMemory, 0, sizeof(Light));

    matrixUniformBufferDescriptorInfos[0].buffer = matrixUniformBuffers[0]->vkBuffer;
    matrixUniformBufferDescriptorInfos[0].offset = 0;
    matrixUniformBufferDescriptorInfos[0].range = sizeof(UBO);
    matrixUniformBufferDescriptorInfos[1].buffer = matrixUniformBuffers[1]->vkBuffer;
    matrixUniformBufferDescriptorInfos[1].offset = 0;
    matrixUniformBufferDescriptorInfos[1].range = sizeof(UBO);

    lightUniformBufferDescriptorInfo.buffer = lightUniformBuffer->vkBuffer;
    lightUniformBufferDescriptorInfo.offset = 0;
    lightUniformBufferDescriptorInfo.range = sizeof(Light);
}

void SwapchainFrame::WriteDescriptorSet(const vk::Device& vkDevice)
{
    vk::WriteDescriptorSet matrixWriteInfo(descriptorSets[0], 0, 0, 2, vk::DescriptorType::eUniformBuffer, nullptr, matrixUniformBufferDescriptorInfos.data(), nullptr, nullptr);
    vk::WriteDescriptorSet lightWriteInfo(descriptorSets[0], 1, 0, vk::DescriptorType::eUniformBuffer, nullptr, lightUniformBufferDescriptorInfo);

    vkDevice.updateDescriptorSets(matrixWriteInfo, nullptr);
    vkDevice.updateDescriptorSets(lightWriteInfo, nullptr);
}
