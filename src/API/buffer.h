#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "device.h"
#include "memory.h"

class Buffer : public Memory
{
    vk::Buffer bufferHandle_;
    vk::DescriptorBufferInfo bufferInfo_;
    size_t bufferSize_;

public:
    Buffer(const BufferInput& bufferInput);
    void MapMemory(vk::DeviceSize range);
    const vk::Buffer& GetBufferHandle() { return bufferHandle_; }
    const vk::DescriptorBufferInfo& GetBufferInfo() { return bufferInfo_; }
    size_t GetBufferSize() { return bufferSize_; }
    const vk::DeviceMemory& GetBufferMemory() { return GetMemory(); }
    void Destroy();
    ~Buffer();

    template <typename T>
    void CopyToBuffer(T resource, const BufferInput& bufferInput)
    {
        memoryLocation_ = Device::GetHandle().device.mapMemory(vkDeviceMemory_, 0, bufferInput.size);
        memcpy(memoryLocation_, resource, bufferInput.size);
        Device::GetHandle().device.unmapMemory(vkDeviceMemory_);
    }

    template <typename T>
    void UpdateBuffer(T resource, size_t size)
    {
        memcpy(memoryLocation_, resource, size);
    }
};

#endif