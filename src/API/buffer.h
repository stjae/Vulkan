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
    Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const BufferInput& bufferInput);
    void MapUniformBuffer();
    vk::Buffer GetBuffer() { return vkBuffer_; }
    vk::DescriptorBufferInfo GetBufferInfo() { return descriptorBufferInfo_; }
    void DestroyBuffer();
    ~Buffer();

    template <typename T>
    void CopyResource(T resource, const BufferInput& bufferInput)
    {
        memoryLocation_ = vkDevice_.mapMemory(vkDeviceMemory_, 0, bufferInput.size);
        memcpy(memoryLocation_, resource, bufferInput.size);
        vkDevice_.unmapMemory(vkDeviceMemory_);
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

    const vk::PhysicalDevice& vkPhysicalDevice_;
    const vk::Device& vkDevice_;
};

#endif