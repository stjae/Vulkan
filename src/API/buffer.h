#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "device.h"
#include "memory.h"

class Buffer : public Memory
{
    BufferHandle handle_;
    size_t bufferSize_;

public:
    Buffer(const BufferInput& bufferInput);
    void MapMemory(vk::DeviceSize range);
    size_t GetSize() { return bufferSize_; }
    void Destroy();
    ~Buffer();

    const BufferHandle& GetHandle() { return handle_; }

    template <typename T>
    void CopyToBuffer(T resource, const BufferInput& bufferInput)
    {
        void* bufferMemoryAddress = Device::GetHandle().device.mapMemory(Memory::GetHandle(), 0, bufferInput.size);

        Memory::SetAddress(bufferMemoryAddress);
        memcpy(bufferMemoryAddress, resource, bufferInput.size);
        Device::GetHandle().device.unmapMemory(Memory::GetHandle());
    }

    template <typename T>
    void UpdateBuffer(T resource, size_t size)
    {
        void* bufferMemoryAddress = Memory::GetAddress();

        memcpy(bufferMemoryAddress, resource, size);
    }
};

#endif