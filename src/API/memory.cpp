#include "memory.h"

uint32_t Memory::FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = Device::GetHandle().physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

        bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

        bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

        if (supported && sufficient) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

void Memory::AllocateMemory(const vk::Buffer& vkBuffer, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements = Device::GetHandle().device.getBufferMemoryRequirements(vkBuffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);

    handle_ = Device::GetHandle().device.allocateMemory(allocateInfo);
    Device::GetHandle().device.bindBufferMemory(vkBuffer, handle_, 0);
}

void Memory::AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements;
    Device::GetHandle().device.getImageMemoryRequirements(vkImage, &memoryRequirements);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);

    handle_ = Device::GetHandle().device.allocateMemory(allocateInfo);
    Device::GetHandle().device.bindImageMemory(vkImage, handle_, 0);
}

void Memory::Free()
{
    if (handle_ != VK_NULL_HANDLE) {
        Device::GetHandle().device.freeMemory(handle_);
        handle_ = VK_NULL_HANDLE;
    }
}