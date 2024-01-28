#ifndef BUFFER_H
#define BUFFER_H

#include "device.h"
#include "memory.h"

struct BufferBundle
{
    vk::Buffer buffer;
    vk::DescriptorBufferInfo bufferInfo;
    vk::DeviceMemory bufferMemory;
};

class Buffer : public Memory
{
    BufferBundle bufferBundle_;
    size_t bufferSize_;

public:
    explicit Buffer(const BufferInput& bufferInput);
    void MapMemory(vk::DeviceSize range);
    size_t GetSize() const { return bufferSize_; }
    void Destroy();
    ~Buffer();

    const BufferBundle& GetBundle() { return bufferBundle_; }
    static size_t GetDynamicBufferOffset(size_t size);

    template <typename T>
    void CopyToBuffer(T resource, const BufferInput& bufferInput)
    {
        void* bufferMemoryAddress = Device::GetBundle().device.mapMemory(Memory::GetMemoryHandle(), 0, bufferInput.size);

        Memory::SetAddress(bufferMemoryAddress);
        memcpy(bufferMemoryAddress, resource, bufferInput.size);
        Device::GetBundle().device.unmapMemory(Memory::GetMemoryHandle());
    }

    template <typename T>
    void UpdateBuffer(T resource, size_t size)
    {
        void* bufferMemoryAddress = Memory::GetAddress();

        memcpy(bufferMemoryAddress, resource, size);
    }
};

#endif