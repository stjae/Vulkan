#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "../common.h"

class Memory
{
public:
    uint32_t FindMemoryTypeIndex(const vk::PhysicalDevice& vkPhysicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    void AllocateMemory(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const vk::Buffer& vkBuffer, vk::MemoryPropertyFlags properties);
    void AllocateMemory(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const vk::Image& vkImage, vk::MemoryPropertyFlags properties);
    void Free(const vk::Device& vkDevice);

protected:
    vk::DeviceMemory vkDeviceMemory_;
    void* memoryLocation_;
};

#endif // __MEMORY_H__