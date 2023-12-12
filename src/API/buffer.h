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
    void Map(vk::DeviceSize range);
    vk::Buffer& GetBuffer() { return vkBuffer_; }
    vk::DescriptorBufferInfo& GetBufferInfo() { return descriptorBufferInfo_; }
    size_t GetBufferSize() { return size_; }
    vk::DeviceMemory& GetBufferMemory() { return GetMemory(); }
    void Destroy();
    ~Buffer();

    template <typename T>
    void CopyToBuffer(T resource, const BufferInput& bufferInput)
    {
        memoryLocation_ =
            Device::GetDevice().mapMemory(vkDeviceMemory_, 0, bufferInput.size);
        memcpy(memoryLocation_, resource, bufferInput.size);
        Device::GetDevice().unmapMemory(vkDeviceMemory_);
    }
    template <typename T>
    void UpdateBuffer(T resource, size_t size)
    {
        memcpy(memoryLocation_, resource, size);
    }

private:
    vk::Buffer vkBuffer_;
    size_t size_;
    vk::DescriptorBufferInfo descriptorBufferInfo_;
};

#endif