#include "image.h"

namespace vkn {
Image::Image()
{
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;

    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    imageViewCreateInfo.subresourceRange = subresourceRange;
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.subresourceRange = subresourceRange;
}

void Image::CreateImage(vk::Extent3D&& extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageTiling tiling, vk::MemoryPropertyFlags memoryProperty, vk::Sampler sampler)
{
    imageCreateInfo.extent = extent;
    imageCreateInfo.format = format;
    imageCreateInfo.usage = usage;
    imageCreateInfo.tiling = tiling;

    imageBundle_.image = Device::GetBundle().device.createImage(imageCreateInfo);
    imageBundle_.descriptorImageInfo.sampler = sampler;

    memory.AllocateMemory(imageBundle_.image, memoryProperty);
}

void Image::CreateImageView()
{
    imageViewCreateInfo.image = imageBundle_.image;
    imageViewCreateInfo.format = imageCreateInfo.format;

    imageBundle_.imageView = Device::GetBundle().device.createImageView(imageViewCreateInfo);
    imageBundle_.descriptorImageInfo.imageView = imageBundle_.imageView;
}

void Image::InsertImage(const std::string& filePath, vk::Format format, vk::CommandBuffer& commandBuffer)
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

    BufferInput bufferInput = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    stagingBuffer_ = std::make_shared<Buffer>(bufferInput);
    stagingBuffer_->Copy(imageData);

    stbi_image_free(imageData);

    CreateImage({ (uint32_t)width, (uint32_t)height, 1 }, format, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    CreateImageView();

    Command::Begin(commandBuffer);
    // Set texture image layout to transfer dst optimal
    Command::SetImageMemoryBarrier(commandBuffer,
                                   imageBundle_.image,
                                   imageBundle_.descriptorImageInfo,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTransfer);
    // Copy texture image from staging buffer
    Command::CopyBufferToImage(commandBuffer,
                               stagingBuffer_->GetBundle().buffer,
                               imageBundle_.image, width,
                               height);
    // Set texture image layout to shader read only
    Command::SetImageMemoryBarrier(commandBuffer,
                                   imageBundle_.image,
                                   imageBundle_.descriptorImageInfo,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer.end();
}

// TODO: reuse dummy
void Image::InsertDummyImage(vk::CommandBuffer& commandBuffer, std::array<uint8_t, 4>&& color)
{
    BufferInput bufferInput = { sizeof(color), sizeof(color), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    stagingBuffer_ = std::make_shared<Buffer>(bufferInput);
    stagingBuffer_->Copy(&color);

    CreateImage({ 1, 1, 1 }, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    CreateImageView();

    Command::Begin(commandBuffer);
    // Set texture image layout to transfer dst optimal
    Command::SetImageMemoryBarrier(commandBuffer,
                                   imageBundle_.image,
                                   imageBundle_.descriptorImageInfo,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTransfer);
    // Copy texture image from staging buffer
    Command::CopyBufferToImage(commandBuffer,
                               stagingBuffer_->GetBundle().buffer,
                               imageBundle_.image,
                               1,
                               1);
    // Set texture image layout to shader read only
    Command::SetImageMemoryBarrier(commandBuffer,
                                   imageBundle_.image,
                                   imageBundle_.descriptorImageInfo,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer.end();
}

void Image::InsertHDRImage(const std::string& filePath, vk::Format format, vk::CommandBuffer& commandBuffer)
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

    BufferInput bufferInput = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    Buffer stagingBuffer(bufferInput);
    stagingBuffer.Copy(imageData);

    stbi_image_free(imageData);

    CreateImage({ (uint32_t)width, (uint32_t)height, 1 }, format, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::clampSampler);
    CreateImageView();

    Command::Begin(commandBuffer);
    // Set texture image layout to transfer dst optimal
    Command::SetImageMemoryBarrier(commandBuffer,
                                   imageBundle_.image,
                                   imageBundle_.descriptorImageInfo,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTransfer);
    // Copy texture image from staging buffer
    Command::CopyBufferToImage(commandBuffer,
                               stagingBuffer.GetBundle().buffer,
                               imageBundle_.image, width,
                               height);
    // Set texture image layout to shader read only
    Command::SetImageMemoryBarrier(commandBuffer,
                                   imageBundle_.image,
                                   imageBundle_.descriptorImageInfo,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer.end();
    Command::Submit(&commandBuffer, 1);
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
    samplerInfo.maxAnisotropy = Device::physicalDeviceLimits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable = vk::False;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    CheckResult(Device::GetBundle().device.createSampler(&samplerInfo, nullptr, &repeatSampler));

    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    CheckResult(Device::GetBundle().device.createSampler(&samplerInfo, nullptr, &clampSampler));
}

void Image::DestroyImage()
{
    Device::GetBundle().device.destroyImage(imageBundle_.image);
    imageBundle_.image = nullptr;
}

void Image::DestroyImageView()
{
    Device::GetBundle().device.destroyImageView(imageBundle_.imageView);
    imageBundle_.imageView = nullptr;
}

void Image::CreateFramebuffer(const Pipeline& pipeline)
{
    vk::ImageView attachment = imageBundle_.imageView;
    vk::FramebufferCreateInfo frameBufferCI({}, pipeline.renderPass, 1, &attachment, imageCreateInfo.extent.width, imageCreateInfo.extent.height, 1);

    CheckResult(vkn::Device::GetBundle().device.createFramebuffer(&frameBufferCI, nullptr, &framebuffer_));
}

void Image::Draw(const Mesh& square, const Pipeline& pipeline, vk::CommandBuffer& commandBuffer)
{
    Command::Begin(commandBuffer);
    Command::SetImageMemoryBarrier(commandBuffer,
                                   imageBundle_.image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   {},
                                   vk::AccessFlagBits::eColorAttachmentWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eAllCommands,
                                   imageViewCreateInfo.subresourceRange);
    commandBuffer.end();
    Command::Submit(&commandBuffer, 1);

    Command::Begin(commandBuffer);
    vk::Viewport viewport({}, {}, (float)imageCreateInfo.extent.width, (float)imageCreateInfo.extent.height, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { imageCreateInfo.extent.width, imageCreateInfo.extent.height });
    commandBuffer.setScissor(0, 1, &scissor);

    vk::ClearValue clearValue = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    vk::RenderPassBeginInfo renderPassBI(pipeline.renderPass, framebuffer_, { { 0, 0 }, { imageCreateInfo.extent.width, imageCreateInfo.extent.height } }, 1, &clearValue);

    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);

    vk::DeviceSize vertexOffsets[]{ 0 };
    commandBuffer.bindVertexBuffers(0, 1, &square.vertexBuffers[0]->GetBundle().buffer, vertexOffsets);
    commandBuffer.bindIndexBuffer(square.indexBuffers[0]->GetBundle().buffer, 0, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(square.GetIndicesCount(0), square.GetInstanceCount(), 0, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
    Command::Submit(&commandBuffer, 1);

    Command::Begin(commandBuffer);
    Command::SetImageMemoryBarrier(commandBuffer,
                                   imageBundle_.image,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eColorAttachmentWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eAllCommands,
                                   vk::PipelineStageFlagBits::eAllCommands,
                                   imageViewCreateInfo.subresourceRange);
    commandBuffer.end();
    Command::Submit(&commandBuffer, 1);

    imageBundle_.descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

Image::~Image()
{
    memory.Free();
    if (imageBundle_.image != VK_NULL_HANDLE)
        Device::GetBundle().device.destroyImage(imageBundle_.image);
    if (imageBundle_.imageView != VK_NULL_HANDLE)
        Device::GetBundle().device.destroyImageView(imageBundle_.imageView);
    if (framebuffer_ != VK_NULL_HANDLE)
        Device::GetBundle().device.destroyFramebuffer(framebuffer_);
}

/*
void vkn::Image::InsertImageArrays(const std::array<std::string, 2>& filePaths, vk::CommandBuffer& commandBuffer)
{
    std::array<uint8_t, 4> dummyTexture = { 0, 0, 0, 255 };

    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.arrayLayers = 2;
    CreateImage({ 4096, 4096, 1 }, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    imageViewCreateInfo.viewType = vk::ImageViewType::e2DArray;
    imageViewCreateInfo.subresourceRange.layerCount = 2;
    CreateImageView();

    for (uint32_t i = 0; i < 2; i++) {
        int width, height, channel;
        bool noTexture = false;
        stbi_uc* imageData = stbi_load(filePaths[i].c_str(), &width, &height, &channel, STBI_rgb_alpha);
        vk::DeviceSize imageSize = width * height * 4;

        if (imageData) {
            spdlog::info("load texture from [{}]", filePaths[i].c_str());
        } else {
            width = 1, height = 1, imageSize = 4;
            imageData = dummyTexture.data();
            noTexture = true;
            spdlog::error("failed to load texture");
        }

        BufferInput bufferInput = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        Buffer stagingBuffer(bufferInput);
        stagingBuffer.Copy(imageData);

        if (!noTexture)
            stbi_image_free(imageData);

        Command::Begin(commandBuffer);
        // Set texture image layout to transfer dst optimal
        Command::SetImageMemoryBarrier(commandBuffer,
                                       imageBundle_.image,
                                       imageBundle_.descriptorImageInfo,
                                       vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eTransferDstOptimal,
                                       {},
                                       vk::AccessFlagBits::eTransferWrite,
                                       vk::PipelineStageFlagBits::eTopOfPipe,
                                       vk::PipelineStageFlagBits::eTransfer,
                                       { vk::ImageAspectFlagBits::eColor, 0, 1, i, 1 });
        // Copy texture image from staging buffer
        vkn::Command::CopyBufferToImage(commandBuffer,
                                        stagingBuffer.GetBundle().buffer,
                                        imageBundle_.image, width,
                                        height,
                                        i);
        // Set texture image layout to shader read only
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            imageBundle_.image,
                                            imageBundle_.descriptorImageInfo,
                                            vk::ImageLayout::eTransferDstOptimal,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            vk::AccessFlagBits::eTransferWrite,
                                            vk::AccessFlagBits::eShaderRead,
                                            vk::PipelineStageFlagBits::eTransfer,
                                            vk::PipelineStageFlagBits::eFragmentShader,
                                            { vk::ImageAspectFlagBits::eColor, 0, 1, i, 1 });
        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }
}
*/
} // namespace vkn