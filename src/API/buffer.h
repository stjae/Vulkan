#ifndef BUFFER_H
#define BUFFER_H

#include "device/device.h"
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
        void* bufferMemoryAddress = Device::GetBundle().device.mapMemory(Memory::Get(), 0, bufferInput.size);

        Memory::SetAddress(bufferMemoryAddress);
        memcpy(bufferMemoryAddress, resource, bufferInput.size);
        Device::GetBundle().device.unmapMemory(Memory::Get());
    }

    template <typename T>
    void UpdateBuffer(T resource, size_t size)
    {
        void* bufferMemoryAddress = Memory::GetAddress();

        memcpy(bufferMemoryAddress, resource, size);
    }
};

#endif