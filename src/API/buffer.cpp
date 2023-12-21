#include "buffer.h"
size_t dynamicBufferAlignment;

Buffer::Buffer(const BufferInput& bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage,
                                    vk::SharingMode::eExclusive);

    handle_.buffer = Device::GetHandle().device.createBuffer(bufferInfo);
    bufferSize_ = bufferInput.size;

    AllocateMemory(handle_.buffer, bufferInput.properties);
}

void Buffer::MapMemory(vk::DeviceSize range)
{
    void* bufferMemoryAddress = Device::GetHandle().device.mapMemory(Memory::GetHandle(), 0, range);
    Memory::SetAddress(bufferMemoryAddress);
    handle_.bufferMemory = Memory::GetHandle(); 

    handle_.bufferInfo.buffer = handle_.buffer;
    handle_.bufferInfo.offset = 0;
    handle_.bufferInfo.range = range;
}

void Buffer::Destroy()
{
    Device::GetHandle().device.waitIdle();
    if (handle_.buffer != VK_NULL_HANDLE) {
        Device::GetHandle().device.destroyBuffer(handle_.buffer);
        handle_.buffer = VK_NULL_HANDLE;
    }
    if (Memory::GetHandle() != VK_NULL_HANDLE) {
        Device::GetHandle().device.freeMemory(Memory::GetHandle());
        Memory::SetHandle(VK_NULL_HANDLE);
    }
    Log(debug, fmt::v9::terminal_color::bright_yellow, "buffer destroyed");
}

Buffer::~Buffer() { Destroy(); }
