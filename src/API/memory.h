#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "../common.h"

class Memory
{
public:
    uint32_t FindMemoryTypeIndex(const vk::PhysicalDevice& vkPhysicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    virtual void AllocateMemory(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, vk::MemoryPropertyFlags properties) = 0;

    vk::DeviceMemory vkDeviceMemory;
};

#endif // __MEMORY_H__