#include "image.h"

void Image::CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent)
{
    vk::ImageCreateInfo createInfo({}, vk::ImageType::e2D, format, extent, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, usage);

    handle_.image = Device::GetHandle().device.createImage(createInfo);
    handle_.format = format;
    memory.AllocateMemory(handle_.image, properties);
}

void Image::CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageSubresourceRange range(aspectFlags, 0, 1, 0, 1);
    vk::ImageViewCreateInfo createInfo({}, handle_.image, vk::ImageViewType::e2D, format, {}, range);

    handle_.imageView = Device::GetHandle().device.createImageView(createInfo);
}

void Image::CreateSampler()
{
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = vk::True;
    samplerInfo.maxAnisotropy = Device::limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable = vk::False;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (Device::GetHandle().device.createSampler(&samplerInfo, nullptr, &handle_.sampler) != vk::Result::eSuccess) {
        spdlog::error("failed to create sampler");
    }
}

void Image::SetInfo(vk::ImageLayout imageLayout)
{
    vk::DescriptorImageInfo imageInfo(handle_.sampler, handle_.imageView, imageLayout);
    handle_.imageInfo = imageInfo;
}

void Image::DestroyImage()
{
    Device::GetHandle().device.destroyImage(handle_.image);
    handle_.image = VK_NULL_HANDLE;
}

void Image::DestroyImageView()
{
    Device::GetHandle().device.destroyImageView(handle_.imageView);
    handle_.imageView = VK_NULL_HANDLE;
}

Image::~Image()
{
    memory.Free();
    if (handle_.image != VK_NULL_HANDLE)
        Device::GetHandle().device.destroyImage(handle_.image);
    if (handle_.imageView != VK_NULL_HANDLE)
        Device::GetHandle().device.destroyImageView(handle_.imageView);
    if (handle_.sampler != VK_NULL_HANDLE)
        Device::GetHandle().device.destroySampler(handle_.sampler);
}