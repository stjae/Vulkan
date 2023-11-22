#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "device.h"
#include "memory.h"

struct BufferInput {

    size_t size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};

class Buffer
{
public:
    Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const BufferInput& bufferInput);
    void MapUniformBuffer();
    void DestroyBuffer();
    ~Buffer();

    vk::Buffer vkBuffer;
    Memory memory;
    size_t size;

    vk::DescriptorBufferInfo descriptorBufferInfo;

private:
    const vk::PhysicalDevice& vkPhysicalDevice;
    const vk::Device& vkDevice;
};

#endif