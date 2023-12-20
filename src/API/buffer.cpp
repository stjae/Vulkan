#include "buffer.h"
size_t dynamicBufferAlignment;

Buffer::Buffer(const BufferInput& bufferInput)
{
    vk::BufferCreateInfo bufferInfo({}, bufferInput.size, bufferInput.usage,
                                    vk::SharingMode::eExclusive);

    bufferHandle_ = Device::GetHandle().device.createBuffer(bufferInfo);
    bufferSize_ = bufferInput.size;

    AllocateMemory(bufferHandle_, bufferInput.properties);
}

void Buffer::MapMemory(vk::DeviceSize range)
{
    memoryLocation_ = Device::GetHandle().device.mapMemory(vkDeviceMemory_, 0, range);

    bufferInfo_.buffer = bufferHandle_;
    bufferInfo_.offset = 0;
    bufferInfo_.range = range;
}

void Buffer::Destroy()
{
    Device::GetHandle().device.waitIdle();
    if (bufferHandle_ != VK_NULL_HANDLE) {
        Device::GetHandle().device.destroyBuffer(bufferHandle_);
        bufferHandle_ = VK_NULL_HANDLE;
    }
    if (vkDeviceMemory_ != VK_NULL_HANDLE) {
        Device::GetHandle().device.freeMemory(vkDeviceMemory_);
        vkDeviceMemory_ = VK_NULL_HANDLE;
    }
    Log(debug, fmt::v9::terminal_color::bright_yellow, "buffer destroyed");
}

Buffer::~Buffer() { Destroy(); }
