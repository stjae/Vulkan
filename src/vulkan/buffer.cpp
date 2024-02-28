#include "buffer.h"

Buffer::Buffer(const BufferInput& bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage,
                                    vk::SharingMode::eExclusive);

    bufferBundle_.buffer = Device::GetBundle().device.createBuffer(bufferInfo);
    bufferBundle_.bufferUsage = bufferInput.usage;
    bufferSize_ = bufferInput.size;

    bufferBundle_.bufferInfo.buffer = bufferBundle_.buffer;
    bufferBundle_.bufferInfo.range = bufferInput.size;

    AllocateMemory(bufferBundle_.buffer, bufferInput.properties);
}

void Buffer::MapMemory(vk::DeviceSize range)
{
    void* bufferMemoryAddress = Device::GetBundle().device.mapMemory(Memory::GetMemory(), 0, range);
    Memory::SetAddress(bufferMemoryAddress);
    bufferBundle_.bufferMemory = Memory::GetMemory();

    bufferBundle_.bufferInfo.buffer = bufferBundle_.buffer;
    bufferBundle_.bufferInfo.offset = 0;
    bufferBundle_.bufferInfo.range = range;
}

void Buffer::Destroy()
{
    Device::GetBundle().device.waitIdle();
    if (bufferBundle_.buffer != VK_NULL_HANDLE) {
        Device::GetBundle().device.destroyBuffer(bufferBundle_.buffer);
        bufferBundle_.buffer = nullptr;
    }
    if (Memory::GetMemory() != VK_NULL_HANDLE) {
        Device::GetBundle().device.freeMemory(Memory::GetMemory());
        Memory::Set(nullptr);
    }
    Log(debug, fmt::terminal_color::bright_yellow, "buffer destroyed {}", to_string(bufferBundle_.bufferUsage));
}

size_t Buffer::GetDynamicBufferRange(size_t size)
{
    // offset should be multiple of minOffset
    size_t minOffset = Device::physicalDeviceLimits.minUniformBufferOffsetAlignment;
    size_t dynamicBufferOffset = 2;

    while (dynamicBufferOffset < size || dynamicBufferOffset < minOffset) {
        dynamicBufferOffset *= 2;
    }

    return dynamicBufferOffset;
}

Buffer::~Buffer() { Destroy(); }
