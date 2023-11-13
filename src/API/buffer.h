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
    Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice) : vkPhysicalDevice(vkPhysicalDevice), vkDevice(vkDevice) {}
    void CreateBuffer(const BufferInput& bufferInput);
    ~Buffer();

    vk::Buffer vkBuffer;
    Memory memory;

private:
    const vk::PhysicalDevice& vkPhysicalDevice;
    const vk::Device& vkDevice;
};

#endif