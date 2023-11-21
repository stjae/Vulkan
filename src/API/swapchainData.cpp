#include "swapchainData.h"

void SwapchainFrame::CreateUniformBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    vk::PhysicalDeviceProperties props = vkPhysicalDevice.getProperties();
    vk::DeviceSize minOffset = props.limits.minUniformBufferOffsetAlignment;

    BufferInput input;
    input.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    input.size = minOffset * 2;
    input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
    matrixUniformBuffer = std::make_shared<Buffer>(vkPhysicalDevice, vkDevice);
    matrixUniformBuffer->CreateBuffer(input);

    matrixUniformBufferMemoryLocation = vkDevice.mapMemory(matrixUniformBuffer->memory.vkDeviceMemory, 0, minOffset * 2);

    input.size = sizeof(Light);
    lightUniformBuffer = std::make_shared<Buffer>(vkPhysicalDevice, vkDevice);
    lightUniformBuffer->CreateBuffer(input);

    lightUniformBufferMemoryLocation = vkDevice.mapMemory(lightUniformBuffer->memory.vkDeviceMemory, 0, sizeof(Light));

    matrixUniformBufferDescriptorInfo.buffer = matrixUniformBuffer->vkBuffer;
    matrixUniformBufferDescriptorInfo.offset = 0;
    matrixUniformBufferDescriptorInfo.range = VK_WHOLE_SIZE;

    lightUniformBufferDescriptorInfo.buffer = lightUniformBuffer->vkBuffer;
    lightUniformBufferDescriptorInfo.offset = 0;
    lightUniformBufferDescriptorInfo.range = sizeof(Light);
}

void SwapchainFrame::WriteDescriptorSet(const vk::Device& vkDevice)
{
    std::array<vk::WriteDescriptorSet, 2> matrixWriteInfos;
    vk::WriteDescriptorSet set0(descriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, matrixUniformBufferDescriptorInfo);
    matrixWriteInfos[0] = set0;
    vk::WriteDescriptorSet set1(descriptorSets[0], 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, matrixUniformBufferDescriptorInfo);
    matrixWriteInfos[1] = set1;
    vk::WriteDescriptorSet lightWriteInfo(descriptorSets[0], 1, 0, vk::DescriptorType::eUniformBuffer, nullptr, lightUniformBufferDescriptorInfo);

    vkDevice.updateDescriptorSets(matrixWriteInfos, nullptr);
    vkDevice.updateDescriptorSets(lightWriteInfo, nullptr);
}
