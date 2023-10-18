#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../common.h"

struct BufferInput {

    size_t size;
    vk::BufferUsageFlags usage;
};

struct Buffer {

    vk::Buffer buffer;
    vk::DeviceMemory bufferMemory;
};

inline Buffer CreateBuffer(BufferInput input)
{

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.setSize(input.size);
}

#endif