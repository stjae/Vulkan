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

void Image::TransitImageLayout(vk::CommandBuffer& commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {});
    commandBuffer.begin(beginInfo);

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    vk::ImageMemoryBarrier barrier;
    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
        spdlog::error("unsupported layout transition");
    }
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    commandBuffer.pipelineBarrier(srcStage, dstStage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
    commandBuffer.end();
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

Image::~Image()
{
    vkDevice.freeMemory(memory.vkDeviceMemory);
    vkDevice.destroyImage(image);
    vkDevice.destroySampler(sampler);
    vkDevice.destroyImageView(imageView);
}