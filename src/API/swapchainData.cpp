#include "swapchainData.h"

void SwapchainFrame::CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    BufferInput input;
    input.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    input.size = sizeof(UBO);
    input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
    matrixUniformBuffer = std::make_shared<Buffer>(vkPhysicalDevice, vkDevice, input);

    input.size = sizeof(Light);
    lightUniformBuffer = std::make_shared<Buffer>(vkPhysicalDevice, vkDevice, input);

    matrixUniformBufferMemoryLocation = vkDevice.mapMemory(matrixUniformBuffer->vkDeviceMemory, 0, sizeof(UBO));
    lightUniformBufferMemoryLocation = vkDevice.mapMemory(lightUniformBuffer->vkDeviceMemory, 0, sizeof(Light));

    matrixUniformBufferDescriptorInfo.buffer = matrixUniformBuffer->vkBuffer;
    matrixUniformBufferDescriptorInfo.offset = 0;
    matrixUniformBufferDescriptorInfo.range = sizeof(UBO);

    lightUniformBufferDescriptorInfo.buffer = lightUniformBuffer->vkBuffer;
    lightUniformBufferDescriptorInfo.offset = 0;
    lightUniformBufferDescriptorInfo.range = sizeof(Light);
}

void SwapchainFrame::WriteDescriptorSet(const vk::Device& vkDevice)
{
    vk::WriteDescriptorSet matrixWriteInfo(descriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, matrixUniformBufferDescriptorInfo);
    vk::WriteDescriptorSet lightWriteInfo(descriptorSets[0], 1, 0, vk::DescriptorType::eUniformBuffer, nullptr, lightUniformBufferDescriptorInfo);

    vkDevice.updateDescriptorSets(matrixWriteInfo, nullptr);
    vkDevice.updateDescriptorSets(lightWriteInfo, nullptr);
}

void Image::AllocateMemory(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memoryRequirements;
    vkDevice.getImageMemoryRequirements(image, &memoryRequirements);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryTypeIndex(vkPhysicalDevice, memoryRequirements.memoryTypeBits, properties);

    vkDeviceMemory = vkDevice.allocateMemory(allocateInfo);
    vkDevice.bindImageMemory(image, vkDeviceMemory, 0);
}

Image::~Image()
{
    vkDevice.freeMemory(vkDeviceMemory);
    vkDevice.destroyImage(image);
    vkDevice.destroyImageView(imageView);
}

void SwapchainFrame::CreateImage(Image& image, const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent)
{
    vk::ImageCreateInfo createInfo({}, vk::ImageType::e2D, format, extent, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usage);

    image.image = vkDevice.createImage(createInfo);
    image.format = format;
    image.AllocateMemory(vkPhysicalDevice, vkDevice, properties);
}

void SwapchainFrame::CreateImageView(Image& image, const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageSubresourceRange range(aspectFlags, 0, 1, 0, 1);
    vk::ImageViewCreateInfo createInfo({}, image.image, vk::ImageViewType::e2D, format, {}, range);

    image.imageView = vkDevice.createImageView(createInfo);
}
