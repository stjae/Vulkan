#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "device.h"
#include "memory.h"

struct BufferInput {

    size_t size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};

class Buffer : public Memory
{
public:
    Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const BufferInput& input);
    void AllocateMemory(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, vk::MemoryPropertyFlags properties) override;
    ~Buffer();

    vk::Buffer vkBuffer;

private:
    const vk::PhysicalDevice& vkPhysicalDevice;
    const vk::Device& vkDevice;
    const BufferInput& bufferInput;
};

#endif