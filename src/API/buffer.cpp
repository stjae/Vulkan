#include "buffer.h"

Buffer::Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const BufferInput& bufferInput) : vkPhysicalDevice(vkPhysicalDevice), vkDevice(vkDevice), bufferInput(bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage, vk::SharingMode::eExclusive);

    vkBuffer = vkDevice.createBuffer(bufferInfo);

    AllocateMemory(vkPhysicalDevice, vkDevice, bufferInput.properties);
}

void Buffer::AllocateMemory(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements = vkDevice.getBufferMemoryRequirements(vkBuffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(vkPhysicalDevice, memoryRequirements.memoryTypeBits, bufferInput.properties);

    vkDeviceMemory = vkDevice.allocateMemory(allocateInfo);
    vkDevice.bindBufferMemory(vkBuffer, vkDeviceMemory, 0);
}

Buffer::~Buffer()
{
    vkDevice.waitIdle();
    vkDevice.destroyBuffer(vkBuffer);
    vkDevice.freeMemory(vkDeviceMemory);
}
