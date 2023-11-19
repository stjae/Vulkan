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

void Image::CreateSampler(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = vk::True;
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable = vk::False;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkDevice.createSampler(&samplerInfo, nullptr, &sampler) != vk::Result::eSuccess) {
        spdlog::error("failed to create sampler");
    }
}

void Image::SetTextureImageInfo(vk::ImageLayout imageLayout)
{
    vk::DescriptorImageInfo imageInfo(sampler, imageView, imageLayout);
    this->imageInfo = imageInfo;
}

Image::~Image()
{
    vkDevice.freeMemory(memory.vkDeviceMemory);
    vkDevice.destroyImage(image);
    vkDevice.destroySampler(sampler);
    vkDevice.destroyImageView(imageView);
}