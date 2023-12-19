#include "buffer.h"
size_t dynamicBufferAlignment;

Buffer::Buffer(const BufferInput& bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage,
                                    vk::SharingMode::eExclusive);

    vkBuffer_ = Device::GetDevice().createBuffer(bufferInfo);
    size_ = bufferInput.size;

    AllocateMemory(vkBuffer_, bufferInput.properties);
}

void Buffer::Map(vk::DeviceSize range)
{
    memoryLocation_ = Device::GetDevice().mapMemory(vkDeviceMemory_, 0, range);

    descriptorBufferInfo_.buffer = vkBuffer_;
    descriptorBufferInfo_.offset = 0;
    descriptorBufferInfo_.range = range;
}

void Buffer::Destroy()
{
    Device::GetDevice().waitIdle();
    if (vkBuffer_ != VK_NULL_HANDLE) {
        Device::GetDevice().destroyBuffer(vkBuffer_);
        vkBuffer_ = VK_NULL_HANDLE;
    }
    if (vkDeviceMemory_ != VK_NULL_HANDLE) {
        Device::GetDevice().freeMemory(vkDeviceMemory_);
        vkDeviceMemory_ = VK_NULL_HANDLE;
    }
    Log(debug, fmt::v9::terminal_color::bright_yellow, "buffer destroyed");
}

Buffer::~Buffer() { Destroy(); }
