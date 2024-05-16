#include "memory.h"
#include "device.h"

uint32_t vkn::Memory::FindMemoryTypeIndex(uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
{
    vk::PhysicalDeviceMemoryProperties memoryProperties = vkn::Device::Get().physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

        bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

        bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

        if (supported && sufficient) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable m_memory type");
}
void vkn::Memory::AllocateMemory(const vk::Buffer& buffer, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements = vkn::Device::Get().device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);

    m_memory = vkn::Device::Get().device.allocateMemory(allocateInfo);
    vkn::Device::Get().device.bindBufferMemory(buffer, m_memory, 0);
}
void vkn::Memory::AllocateMemory(const vk::Image& vkImage, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements;
    vkn::Device::Get().device.getImageMemoryRequirements(vkImage, &memoryRequirements);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, properties);

    m_memory = vkn::Device::Get().device.allocateMemory(allocateInfo);
    vkn::Device::Get().device.bindImageMemory(vkImage, m_memory, 0);
}
void vkn::Memory::Free()
{
    if (m_memory != VK_NULL_HANDLE) {
        vkn::Device::Get().device.freeMemory(m_memory);
        m_memory = nullptr;
    }
}