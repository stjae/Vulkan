#include "buffer.h"

void Buffer::CreateBuffer(const BufferInput& bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage, vk::SharingMode::eExclusive);

    vkBuffer = vkDevice.createBuffer(bufferInfo);

    memory.AllocateMemory(vkPhysicalDevice, vkDevice, vkBuffer, bufferInput.properties);
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
