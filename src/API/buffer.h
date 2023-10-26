#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "device.h"

struct BufferInput {

    size_t size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};

class Buffer
{
public:
    Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const BufferInput& input);
    uint32_t FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);
    void AllocateBufferMemory();
    ~Buffer();

    vk::Buffer vkBuffer;
    vk::DeviceMemory vkDeviceMemory;

private:
    const vk::PhysicalDevice& vkPhysicalDevice;
    const vk::Device& vkDevice;
    const BufferInput& bufferInput;
};

#endif