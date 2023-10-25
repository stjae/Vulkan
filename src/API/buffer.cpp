#include "buffer.h"

Buffer::Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const BufferInput& bufferInput) : vkPhysicalDevice(vkPhysicalDevice), vkDevice(vkDevice), bufferInput(bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage, vk::SharingMode::eExclusive);

    vkBuffer = vkDevice.createBuffer(bufferInfo);

    AllocateBufferMemory();
}

uint32_t Buffer::FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = vkPhysicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

        bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

        bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

        if (supported && sufficient) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

void Buffer::AllocateBufferMemory()
{
    vk::MemoryRequirements memoryRequirements = vkDevice.getBufferMemoryRequirements(vkBuffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, bufferInput.properties);

    vkDeviceMemory = vkDevice.allocateMemory(allocateInfo);
    vkDevice.bindBufferMemory(vkBuffer, vkDeviceMemory, 0);
}

Buffer::~Buffer()
{
    vkDevice.waitIdle();
    vkDevice.destroyBuffer(vkBuffer);
    vkDevice.freeMemory(vkDeviceMemory);
}
