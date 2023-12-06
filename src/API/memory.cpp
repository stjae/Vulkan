#include "memory.h"

uint32_t Memory::FindMemoryTypeIndex(const vk::PhysicalDevice& vkPhysicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = vkPhysicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

        bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

        bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

        if (supported && sufficient) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

void Memory::AllocateMemory(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const vk::Buffer& vkBuffer, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements = vkDevice.getBufferMemoryRequirements(vkBuffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(vkPhysicalDevice, memoryRequirements.memoryTypeBits, properties);

    vkDeviceMemory_ = vkDevice.allocateMemory(allocateInfo);
    vkDevice.bindBufferMemory(vkBuffer, vkDeviceMemory_, 0);
}

void Memory::AllocateMemory(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, const vk::Image& vkImage, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements;
    vkDevice.getImageMemoryRequirements(vkImage, &memoryRequirements);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(vkPhysicalDevice, memoryRequirements.memoryTypeBits, properties);

    vkDeviceMemory_ = vkDevice.allocateMemory(allocateInfo);
    vkDevice.bindImageMemory(vkImage, vkDeviceMemory_, 0);
}

void Memory::Free(const vk::Device& vkDevice)
{
    vkDevice.freeMemory(vkDeviceMemory_);
}