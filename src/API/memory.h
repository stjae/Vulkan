#ifndef MEMORY_H
#define MEMORY_H

#include "../common.h"
#include "device/device.h"

class Memory
{
    vk::DeviceMemory memory_;
    void* address_{};

protected:
    void Set(vk::DeviceMemory memory) { memory_ = memory; }
    void SetAddress(void* const address) { address_ = address; }

public:
    uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    void AllocateMemory(const vk::Buffer& vkBuffer, vk::MemoryPropertyFlags properties);
    void AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties);
    void Free();

    const vk::DeviceMemory& Get() { return memory_; }
    void* GetAddress() { return address_; }
};

#endif