#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "device.h"
#include "memory.h"

struct BufferInput
{
    size_t size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};

class Buffer : public Memory
{
public:
    Buffer(const BufferInput& bufferInput);
    void MapUniformBuffer();
    vk::Buffer GetBuffer() { return vkBuffer_; }
    vk::DescriptorBufferInfo GetBufferInfo() { return descriptorBufferInfo_; }
    void DestroyBuffer();
    ~Buffer();

    template <typename T>
    void CopyResource(T resource, const BufferInput& bufferInput)
    {
        memoryLocation_ = Device::GetDevice().mapMemory(vkDeviceMemory_, 0, bufferInput.size);
        memcpy(memoryLocation_, resource, bufferInput.size);
        Device::GetDevice().unmapMemory(vkDeviceMemory_);
    }
    template <typename T>
    void UpdateResource(T resource, size_t size)
    {
        memcpy(memoryLocation_, resource, size);
    }

private:
    vk::Buffer vkBuffer_;
    size_t size_;
    vk::DescriptorBufferInfo descriptorBufferInfo_;
};

#endif