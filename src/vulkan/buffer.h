#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include "device.h"
#include "memory.h"

struct BufferInput
{
    size_t size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};
struct BufferBundle
{
    vk::Buffer buffer;
    vk::DescriptorBufferInfo bufferInfo;
    vk::DeviceMemory bufferMemory;
    vk::BufferUsageFlags bufferUsage;
};

class Buffer : public Memory
{
    BufferBundle bufferBundle_;
    size_t bufferSize_;

public:
    Buffer(const BufferInput& bufferInput);
    void MapMemory(vk::DeviceSize range);
    size_t GetSize() const { return bufferSize_; }
    void Destroy();
    ~Buffer();

    const BufferBundle& GetBundle() { return bufferBundle_; }
    static size_t GetDynamicBufferOffset(size_t size);

    template <typename T>
    void CopyToBuffer(T resource, const BufferInput& bufferInput)
    {
        void* bufferMemoryAddress = Device::GetBundle().device.mapMemory(GetMemory(), 0, bufferInput.size);

        SetAddress(bufferMemoryAddress);
        memcpy(bufferMemoryAddress, resource, bufferInput.size);
        Device::GetBundle().device.unmapMemory(GetMemory());
    }

    template <typename T>
    void UpdateBuffer(T resource, size_t size)
    {
        void* bufferMemoryAddress = GetMemoryAddress();

        memcpy(bufferMemoryAddress, resource, size);
    }
};

#endif