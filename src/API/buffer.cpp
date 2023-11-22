#include "buffer.h"

Buffer::Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const BufferInput& bufferInput) : vkPhysicalDevice(vkPhysicalDevice), vkDevice(vkDevice)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage, vk::SharingMode::eExclusive);

    vkBuffer = vkDevice.createBuffer(bufferInfo);

    size = bufferInput.size;

    memory.AllocateMemory(vkPhysicalDevice, vkDevice, vkBuffer, bufferInput.properties);
}

void Buffer::MapUniformBuffer()
{
    memory.memoryLocation = vkDevice.mapMemory(memory.vkDeviceMemory, 0, static_cast<vk::DeviceSize>(size));

    descriptorBufferInfo.buffer = vkBuffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = static_cast<vk::DeviceSize>(size);
}

void Buffer::DestroyBuffer()
{
    vkDevice.waitIdle();
    if (vkBuffer != VK_NULL_HANDLE) {
        vkDevice.destroyBuffer(vkBuffer);
        vkBuffer = VK_NULL_HANDLE;
    }
    if (memory.vkDeviceMemory != VK_NULL_HANDLE) {
        vkDevice.freeMemory(memory.vkDeviceMemory);
        memory.vkDeviceMemory = VK_NULL_HANDLE;
    }
    Log(debug, fmt::v9::terminal_color::bright_yellow, "buffer destroyed");
}

Buffer::~Buffer()
{
    DestroyBuffer();
}
