#include "memory.h"

uint32_t Memory::FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = Device::GetBundle().physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

        bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

        bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

        if (supported && sufficient) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}
void Memory::AllocateMemory(const vk::Buffer& buffer, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements = Device::GetBundle().device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);

    memory_ = Device::GetBundle().device.allocateMemory(allocateInfo);
    Device::GetBundle().device.bindBufferMemory(buffer, memory_, 0);
}
void Memory::AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements;
    Device::GetBundle().device.getImageMemoryRequirements(vkImage, &memoryRequirements);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);

    memory_ = Device::GetBundle().device.allocateMemory(allocateInfo);
    Device::GetBundle().device.bindImageMemory(vkImage, memory_, 0);
}
void Memory::Free()
{
    if (memory_ != VK_NULL_HANDLE) {
        Device::GetBundle().device.freeMemory(memory_);
        memory_ = nullptr;
    }
}