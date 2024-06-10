#include "image.h"

namespace vkn {
Image::Image()
{
    m_imageCreateInfo.imageType = vk::ImageType::e2D;
    m_imageCreateInfo.mipLevels = 1;
    m_imageCreateInfo.arrayLayers = 1;
    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    m_imageViewCreateInfo.subresourceRange = subresourceRange;
    m_imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    m_imageViewCreateInfo.subresourceRange = subresourceRange;
}

void Image::CreateImage(vk::Extent3D&& extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageTiling tiling, vk::MemoryPropertyFlags memoryProperty, vk::Sampler sampler, int mipLevels, vk::SampleCountFlagBits sampleCount)
{
    m_imageCreateInfo.extent = extent;
    m_imageCreateInfo.format = format;
    m_imageCreateInfo.usage = usage;
    m_imageCreateInfo.tiling = tiling;
    m_imageCreateInfo.mipLevels = mipLevels;
    m_imageCreateInfo.samples = sampleCount;
    m_bundle.image = Device::Get().device.createImage(m_imageCreateInfo);
    m_bundle.descriptorImageInfo.sampler = sampler;
    m_memory.AllocateMemory(m_bundle.image, memoryProperty);
    m_imageViewCreateInfo.image = m_bundle.image;
    m_imageViewCreateInfo.format = m_imageCreateInfo.format;
    m_imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
}

void Image::CreateImageView()
{
    m_bundle.imageView = Device::Get().device.createImageView(m_imageViewCreateInfo);
    m_bundle.descriptorImageInfo.imageView = m_bundle.imageView;
}

void Image::InsertImage(const std::string& filePath, vk::Format format, const vk::CommandBuffer& commandBuffer)
{
    stbi_set_flip_vertically_on_load(false);
    int width = 0, height = 0, channel = 0;
    vk::DeviceSize imageSize = 0;
    stbi_uc* imageData = nullptr;

    imageData = stbi_load(filePath.c_str(), &width, &height, &channel, STBI_rgb_alpha);
    imageSize = width * height * 4;

    if (!imageData) {
        spdlog::error("failed to load texture from [{}]", filePath.c_str());
        InsertDummyImage(commandBuffer);
        return;
    }
    spdlog::info("load texture from [{}]", filePath.c_str());

    BufferInfo bufferInfo = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_stagingBuffer = std::make_unique<Buffer>(bufferInfo);
    m_stagingBuffer->Copy(imageData);

    stbi_image_free(imageData);

    CreateImage({ (uint32_t)width, (uint32_t)height, 1 }, format, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    CreateImageView();

    ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    Command::CopyBufferToImage(commandBuffer, m_stagingBuffer->Get().buffer, m_bundle.image, width, height);
    ChangeImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

// TODO: reuse dummy
void Image::InsertDummyImage(const vk::CommandBuffer& commandBuffer, std::array<uint8_t, 4>&& color)
{
    BufferInfo bufferInfo = { sizeof(color), sizeof(color), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_stagingBuffer = std::make_unique<Buffer>(bufferInfo);
    m_stagingBuffer->Copy(&color);

    CreateImage({ 1, 1, 1 }, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    CreateImageView();

    ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    Command::CopyBufferToImage(commandBuffer, m_stagingBuffer->Get().buffer, m_bundle.image, 1, 1);
    ChangeImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void Image::InsertHDRImage(const std::string& filePath, vk::Format format, const vk::CommandBuffer& commandBuffer)
{
    stbi_set_flip_vertically_on_load(true);

    int width = 0, height = 0, channel = 0;
    vk::DeviceSize imageSize = 0;
    float* imageData = nullptr;

    imageData = stbi_loadf(filePath.c_str(), &width, &height, &channel, STBI_rgb_alpha);
    imageSize = width * height * 16;

    if (!imageData) {
        spdlog::error("failed to load texture from [{}]", filePath.c_str());
        InsertDummyImage(commandBuffer, { 32, 32, 32, 255 });
        return;
    }
    spdlog::info("load texture from [{}]", filePath.c_str());

    BufferInfo bufferInfo = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_stagingBuffer = std::make_unique<Buffer>(bufferInfo);
    m_stagingBuffer->Copy(imageData);

    stbi_image_free(imageData);

    CreateImage({ (uint32_t)width, (uint32_t)height, 1 }, format, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_clampSampler);
    CreateImageView();

    ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    Command::CopyBufferToImage(commandBuffer, m_stagingBuffer->Get().buffer, m_bundle.image, width, height);
    ChangeImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
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
    samplerInfo.maxAnisotropy = Device::Get().physicalDevice.getProperties().limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable = vk::False;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    CheckResult(Device::Get().device.createSampler(&samplerInfo, nullptr, &s_repeatSampler));

    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    CheckResult(Device::Get().device.createSampler(&samplerInfo, nullptr, &s_clampSampler));
}

void Image::DestroyImage()
{
    Device::Get().device.destroyImage(m_bundle.image);
    m_bundle.image = nullptr;
}

void Image::DestroyImageView()
{
    Device::Get().device.destroyImageView(m_bundle.imageView);
    m_bundle.imageView = nullptr;
}

void Image::CreateFramebuffer(const Pipeline& pipeline)
{
    vk::ImageView attachment = m_bundle.imageView;
    vk::FramebufferCreateInfo frameBufferCI({}, pipeline.m_renderPass, 1, &attachment, m_imageCreateInfo.extent.width, m_imageCreateInfo.extent.height, 1);

    CheckResult(vkn::Device::Get().device.createFramebuffer(&frameBufferCI, nullptr, &m_framebuffer));
}

void Image::CreateFramebuffer(const Pipeline& pipeline, uint32_t width, uint32_t height)
{
    vk::ImageView attachment = m_bundle.imageView;
    vk::FramebufferCreateInfo frameBufferCI({}, pipeline.m_renderPass, 1, &attachment, width, height, 1);

    CheckResult(vkn::Device::Get().device.createFramebuffer(&frameBufferCI, nullptr, &m_framebuffer));
}

void Image::Draw(const Mesh& square, const Pipeline& pipeline, const vk::CommandBuffer& commandBuffer)
{
    vk::Viewport viewport({}, {}, (float)m_imageCreateInfo.extent.width, (float)m_imageCreateInfo.extent.height, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { m_imageCreateInfo.extent.width, m_imageCreateInfo.extent.height });
    commandBuffer.setScissor(0, 1, &scissor);

    vk::ClearValue clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    vk::RenderPassBeginInfo renderPassBI(pipeline.m_renderPass, m_framebuffer, { { 0, 0 }, { m_imageCreateInfo.extent.width, m_imageCreateInfo.extent.height } }, 1, &clearValue);

    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.m_pipeline);

    vk::DeviceSize vertexOffsets[]{ 0 };
    commandBuffer.bindVertexBuffers(0, 1, &square.m_vertexBuffers[0]->Get().buffer, vertexOffsets);
    commandBuffer.bindIndexBuffer(square.m_indexBuffers[0]->Get().buffer, 0, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(square.GetIndicesCount(0), square.GetInstanceCount(), 0, 0, 0);

    commandBuffer.endRenderPass();
}

void Image::ChangeImageLayout(const vk::CommandBuffer& commandBuffer, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout)
{
    vkn::Command::ChangeImageLayout(commandBuffer, m_bundle.image, oldImageLayout, newImageLayout, m_imageViewCreateInfo.subresourceRange);
    m_bundle.descriptorImageInfo.imageLayout = newImageLayout;
}

Image::~Image()
{
    m_memory.Free();
    if (m_bundle.image != VK_NULL_HANDLE)
        Device::Get().device.destroyImage(m_bundle.image);
    if (m_bundle.imageView != VK_NULL_HANDLE)
        Device::Get().device.destroyImageView(m_bundle.imageView);
    if (m_framebuffer != VK_NULL_HANDLE)
        Device::Get().device.destroyFramebuffer(m_framebuffer);
}
} // namespace vkn