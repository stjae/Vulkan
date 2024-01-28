#include "buffer.h"

Buffer::Buffer(const BufferInput& bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage,
                                    vk::SharingMode::eExclusive);

    bufferBundle_.buffer = Device::GetBundle().device.createBuffer(bufferInfo);
    bufferSize_ = bufferInput.size;

    AllocateMemory(bufferBundle_.buffer, bufferInput.properties);
}

void Buffer::MapMemory(vk::DeviceSize range)
{
    void* bufferMemoryAddress = Device::GetBundle().device.mapMemory(Memory::GetMemoryHandle(), 0, range);
    Memory::SetAddress(bufferMemoryAddress);
    bufferBundle_.bufferMemory = Memory::GetMemoryHandle();

    bufferBundle_.bufferInfo.buffer = bufferBundle_.buffer;
    bufferBundle_.bufferInfo.offset = 0;
    bufferBundle_.bufferInfo.range = range;
}

void Buffer::Destroy()
{
    Device::GetBundle().device.waitIdle();
    if (bufferBundle_.buffer != VK_NULL_HANDLE) {
        Device::GetBundle().device.destroyBuffer(bufferBundle_.buffer);
        bufferBundle_.buffer = VK_NULL_HANDLE;
    }
    if (Memory::GetMemoryHandle() != VK_NULL_HANDLE) {
        Device::GetBundle().device.freeMemory(Memory::GetMemoryHandle());
        Memory::SetMemoryHandle(VK_NULL_HANDLE);
    }
    Log(debug, fmt::v9::terminal_color::bright_yellow, "buffer destroyed");
}

size_t Buffer::GetDynamicBufferOffset(size_t size)
{
    size_t minOffset = Device::physicalDeviceLimits.minUniformBufferOffsetAlignment;
    size_t dynamicBufferOffset = 0;

    if (minOffset > 0) {
        int i = 1;
        while (minOffset * i <= size) {
            dynamicBufferOffset = minOffset * i;
            i++;
        }
    }

    return dynamicBufferOffset;
}

Buffer::~Buffer() { Destroy(); }
