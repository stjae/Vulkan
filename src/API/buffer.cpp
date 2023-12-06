#include "buffer.h"

Buffer::Buffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const BufferInput& bufferInput) : vkPhysicalDevice_(vkPhysicalDevice), vkDevice_(vkDevice)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage, vk::SharingMode::eExclusive);
    vkBuffer_ = vkDevice.createBuffer(bufferInfo);
    size_ = bufferInput.size;

    AllocateMemory(vkPhysicalDevice_, vkDevice_, vkBuffer_, bufferInput.properties);
}

void Buffer::MapUniformBuffer()
{
    memoryLocation_ = vkDevice_.mapMemory(vkDeviceMemory_, 0, static_cast<vk::DeviceSize>(size_));

    descriptorBufferInfo_.buffer = vkBuffer_;
    descriptorBufferInfo_.offset = 0;
    descriptorBufferInfo_.range = static_cast<vk::DeviceSize>(size_);
}

void Buffer::DestroyBuffer()
{
    vkDevice_.waitIdle();
    if (vkBuffer_ != VK_NULL_HANDLE) {
        vkDevice_.destroyBuffer(vkBuffer_);
        vkBuffer_ = VK_NULL_HANDLE;
    }
    if (vkDeviceMemory_ != VK_NULL_HANDLE) {
        vkDevice_.freeMemory(vkDeviceMemory_);
        vkDeviceMemory_ = VK_NULL_HANDLE;
    }
    Log(debug, fmt::v9::terminal_color::bright_yellow, "buffer destroyed");
}

Buffer::~Buffer()
{
    DestroyBuffer();
}
