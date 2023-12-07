#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "../common.h"
#include "device.h"

class Memory
{
public:
    uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    void AllocateMemory(const vk::Buffer& vkBuffer, vk::MemoryPropertyFlags properties);
    void AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties);
    void Free();

protected:
    vk::DeviceMemory vkDeviceMemory_;
    void* memoryLocation_;
};

#endif // __MEMORY_H__