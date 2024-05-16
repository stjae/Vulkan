#ifndef MEMORY_H
#define MEMORY_H

#include "../common.h"
#include "device.h"

namespace vkn {
class Memory
{
    vk::DeviceMemory m_memory;
    void* m_address;

protected:
    void Set(vk::DeviceMemory memory) { m_memory = memory; }
    void SetAddress(void* const address) { m_address = address; }

public:
    uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    void AllocateMemory(const vk::Buffer& buffer, vk::MemoryPropertyFlags properties);
    void AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties);
    void Free();

    const vk::DeviceMemory& GetMemory() { return m_memory; }
    void* GetMemoryAddress() { return m_address; }
};
} // namespace vkn

#endif