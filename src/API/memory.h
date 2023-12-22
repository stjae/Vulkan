#ifndef MEMORY_H
#define MEMORY_H

#include "../common.h"
#include "device.h"

class Memory
{
    vk::DeviceMemory handle_;
    void* memoryAddress_{};

protected:
    const vk::DeviceMemory& GetMemoryHandle() { return handle_; }
    void SetMemoryHandle(vk::DeviceMemory handle) { handle_ = handle; }
    void* GetAddress() { return memoryAddress_; }
    void SetAddress(void* const memoryAddress) { memoryAddress_ = memoryAddress; }

public:
    uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    void AllocateMemory(const vk::Buffer& vkBuffer, vk::MemoryPropertyFlags properties);
    void AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties);
    void Free();
};

#endif