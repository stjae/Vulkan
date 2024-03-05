#include "buffer.h"

Buffer::Buffer(BufferInput bufferInput) : bufferInput_(bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage,
                                    vk::SharingMode::eExclusive);

    bufferBundle_.buffer = Device::GetBundle().device.createBuffer(bufferInfo);
    bufferBundle_.descriptorBufferInfo.buffer = bufferBundle_.buffer;
    bufferBundle_.descriptorBufferInfo.offset = 0;
    bufferBundle_.descriptorBufferInfo.range = bufferInput.range;

    AllocateMemory(bufferBundle_.buffer, bufferInput.properties);
    if (bufferInput.properties & vk::MemoryPropertyFlagBits::eHostVisible)
        MapMemory();
}

void Buffer::MapMemory()
{
    void* bufferMemoryAddress = Device::GetBundle().device.mapMemory(Memory::GetMemory(), 0, bufferInput_.size);
    Memory::SetAddress(bufferMemoryAddress);
    bufferBundle_.bufferMemory = Memory::GetMemory();
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
    Log(logBuffer, fmt::terminal_color::bright_yellow, "buffer destroyed {}", to_string(bufferInput_.usage));
}

Buffer::~Buffer() { Destroy(); }
