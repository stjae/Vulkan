#include "buffer.h"

void Buffer::CreateBuffer(const BufferInput& bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage, vk::SharingMode::eExclusive);

    vkBuffer = vkDevice.createBuffer(bufferInfo);

    memory.AllocateMemory(vkPhysicalDevice, vkDevice, vkBuffer, bufferInput.properties);
}

Buffer::~Buffer()
{
    vkDevice.waitIdle();
    vkDevice.destroyBuffer(vkBuffer);
    vkDevice.freeMemory(memory.vkDeviceMemory);
}
