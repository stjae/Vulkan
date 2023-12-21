#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "../common.h"
#include "device.h"

class Memory
{
    vk::DeviceMemory handle_;
    void* memoryAddress_;

protected:
    const vk::DeviceMemory& GetHandle() { return handle_; }
    void SetHandle(vk::DeviceMemory handle) { handle_ = handle; }
    void* GetAddress() { return memoryAddress_; }
    void SetAddress(void* const memoryAddress) { memoryAddress_ = memoryAddress; }

public:
    uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    void AllocateMemory(const vk::Buffer& vkBuffer, vk::MemoryPropertyFlags properties);
    void AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties);
    void Free();
};

#endif // __MEMORY_H__