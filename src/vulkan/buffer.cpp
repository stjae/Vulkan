#include "buffer.h"
#include "device.h"
#include "memory.h"

vkn::Buffer::Buffer(BufferInfo bufferInfo)
{
    vk::BufferCreateInfo bufferCreateInfo({}, bufferInfo.size, bufferInfo.usage,
                                          vk::SharingMode::eExclusive);

    m_bundle.buffer = vkn::Device::Get().device.createBuffer(bufferCreateInfo);
    m_bundle.bufferInfo = bufferInfo;
    m_bundle.descriptorBufferInfo.buffer = m_bundle.buffer;
    m_bundle.descriptorBufferInfo.offset = 0;
    m_bundle.descriptorBufferInfo.range = bufferInfo.range;

    AllocateMemory(m_bundle.buffer, bufferInfo.properties);
    if (bufferInfo.properties & vk::MemoryPropertyFlagBits::eHostVisible)
        MapMemory();
}

void vkn::Buffer::MapMemory()
{
    void* bufferMemoryAddress = vkn::Device::Get().device.mapMemory(vkn::Memory::GetMemory(), 0, m_bundle.bufferInfo.size);
    Memory::SetAddress(bufferMemoryAddress);
}

void vkn::Buffer::Destroy()
{
    vkn::Device::Get().device.waitIdle();
    if (m_bundle.buffer != VK_NULL_HANDLE) {
        vkn::Device::Get().device.destroyBuffer(m_bundle.buffer);
        m_bundle.buffer = nullptr;
    }
    if (Memory::GetMemory() != VK_NULL_HANDLE) {
        vkn::Device::Get().device.freeMemory(Memory::GetMemory());
        Memory::Set(nullptr);
    }
}

vkn::Buffer::~Buffer() { Destroy(); }
