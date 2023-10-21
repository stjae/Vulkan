#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../common.h"
#include "config.h"

struct BufferInput {

    size_t size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};

struct Buffer {

    vk::Buffer buffer;
    vk::DeviceMemory memory;
};

inline uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

        bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

        bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

        if (supported && sufficient) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

inline void AllocateBufferMemory(Buffer& buffer, const BufferInput& input)
{
    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer.buffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, input.properties);

    buffer.memory = device.allocateMemory(allocateInfo);
    device.bindBufferMemory(buffer.buffer, buffer.memory, 0);
}

inline Buffer CreateBuffer(BufferInput& input)
{
    vk::BufferCreateInfo bufferInfo({}, input.size, input.usage, vk::SharingMode::eExclusive);

    Buffer buffer;
    buffer.buffer = device.createBuffer(bufferInfo);

    AllocateBufferMemory(buffer, input);
    return buffer;
}

#endif