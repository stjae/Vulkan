#ifndef MEMORY_H
#define MEMORY_H

#include "../common.h"
#include "device.h"

namespace vkn {
class Memory
{
    vk::DeviceMemory memory_;
    void* address_{};

protected:
    void Set(vk::DeviceMemory memory) { memory_ = memory; }
    void SetAddress(void* const address) { address_ = address; }

public:
    uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    void AllocateMemory(const vk::Buffer& buffer, vk::MemoryPropertyFlags properties);
    void AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties);
    void Free();

    const vk::DeviceMemory& GetMemory() { return memory_; }
    void* GetMemoryAddress() { return address_; }
};
} // namespace vkn

#endif