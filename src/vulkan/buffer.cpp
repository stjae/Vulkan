#include "buffer.h"
#include "device.h"
#include "memory.h"

vkn::Buffer::Buffer(BufferInput bufferInput) : bufferInput_(bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage,
                                    vk::SharingMode::eExclusive);

    bufferBundle_.buffer = vkn::Device::GetBundle().device.createBuffer(bufferInfo);
    bufferBundle_.descriptorBufferInfo.buffer = bufferBundle_.buffer;
    bufferBundle_.descriptorBufferInfo.offset = 0;
    bufferBundle_.descriptorBufferInfo.range = bufferInput.range;

    AllocateMemory(bufferBundle_.buffer, bufferInput.properties);
    if (bufferInput.properties & vk::MemoryPropertyFlagBits::eHostVisible)
        MapMemory();
}

void vkn::Buffer::MapMemory()
{
    void* bufferMemoryAddress = vkn::Device::GetBundle().device.mapMemory(vkn::Memory::GetMemory(), 0, bufferInput_.size);
    Memory::SetAddress(bufferMemoryAddress);
    bufferBundle_.bufferMemory = Memory::GetMemory();
}

void vkn::Buffer::Destroy()
{
    vkn::Device::GetBundle().device.waitIdle();
    if (bufferBundle_.buffer != VK_NULL_HANDLE) {
        vkn::Device::GetBundle().device.destroyBuffer(bufferBundle_.buffer);
        bufferBundle_.buffer = nullptr;
    }
    if (Memory::GetMemory() != VK_NULL_HANDLE) {
        vkn::Device::GetBundle().device.freeMemory(Memory::GetMemory());
        Memory::Set(nullptr);
    }
}

vkn::Buffer::~Buffer() { Destroy(); }
