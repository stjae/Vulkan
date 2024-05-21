#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include "device.h"
#include "memory.h"

namespace vkn {
struct BufferInfo
{
    size_t size;
    size_t range;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};
class Buffer : public Memory
{
    struct Bundle
    {
        vk::Buffer buffer;
        BufferInfo bufferInfo;
        vk::DescriptorBufferInfo descriptorBufferInfo;
    } m_bundle;
    void MapMemory();

public:
    explicit Buffer(BufferInfo bufferInfo);
    ~Buffer();
    const Bundle& Get() { return m_bundle; }
    void Copy(void* source) { memcpy(GetMemoryAddress(), source, m_bundle.bufferInfo.size); }
    void Destroy();
};
} // namespace vkn

#endif