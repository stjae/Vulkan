#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include "device.h"
#include "memory.h"

namespace vkn {
struct BufferInput
{
    size_t size;
    size_t range;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};
struct BufferBundle
{
    vk::Buffer buffer;
    vk::DeviceMemory bufferMemory;
    vk::DescriptorBufferInfo descriptorBufferInfo;
};
class Buffer : public vkn::Memory
{
    const BufferInput bufferInput_;
    BufferBundle bufferBundle_;

    void MapMemory();

public:
    explicit Buffer(BufferInput bufferInput);
    void Copy(void* source) { memcpy(GetMemoryAddress(), source, bufferInput_.size); }
    const BufferInput& GetBufferInput() const { return bufferInput_; }
    void Destroy();
    ~Buffer();

    const BufferBundle& GetBundle() { return bufferBundle_; }
};
} // namespace vkn

#endif