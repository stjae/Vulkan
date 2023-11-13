#include "image.h"

void Image::CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent)
{
    vk::ImageCreateInfo createInfo({}, vk::ImageType::e2D, format, extent, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usage);

    image = vkDevice.createImage(createInfo);
    this->format = format;
    memory.AllocateMemory(vkPhysicalDevice, vkDevice, image, properties);
}

void Image::CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageSubresourceRange range(aspectFlags, 0, 1, 0, 1);
    vk::ImageViewCreateInfo createInfo({}, image, vk::ImageViewType::e2D, format, {}, range);

    imageView = vkDevice.createImageView(createInfo);
}

Image::~Image()
{
    vkDevice.freeMemory(memory.vkDeviceMemory);
    vkDevice.destroyImage(image);
    vkDevice.destroyImageView(imageView);
}