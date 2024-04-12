#include "prefilteredCubemap.h"

void PrefilteredCubemap::CreateMipmap(vk::CommandBuffer& commandBuffer)
{
    uint32_t mipMapSize = 64;

    mipmap.imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    mipmap.imageCreateInfo.imageType = vk::ImageType::e2D;
    mipmap.imageCreateInfo.mipLevels = numMips_;
    mipmap.imageCreateInfo.arrayLayers = 6;
    mipmap.CreateImage({ mipMapSize, mipMapSize, 1 }, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, numMips_, 0, 6);
    mipmap.imageViewCreateInfo.subresourceRange = subresourceRange;
    mipmap.imageViewCreateInfo.viewType = vk::ImageViewType::eCube;
    mipmap.CreateImageView();

    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        mipmap.GetBundle().image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eHostWrite,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        mipmap.imageViewCreateInfo.subresourceRange);
    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);

    CreateMipmapSampler();

    mipmapDescriptorImageInfo.imageView = mipmap.GetBundle().imageView;
    mipmapDescriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    mipmapDescriptorImageInfo.sampler = mipmapSampler_;
}

void PrefilteredCubemap::CreatePrefilteredCubemap(int numMips, uint32_t cubemapSize, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    numMips_ = numMips;
    CreateMipmap(commandBuffer);

    imageSize_ = cubemapSize;
    imageCreateInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc;
    CreateCubemap(imageSize_, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc, commandBuffer);

    for (int i = 0; i < 6; i++) {
        vkn::Command::Begin(commandBuffer);
        imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            imageBundle_.image,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            vk::ImageLayout::eColorAttachmentOptimal,
                                            vk::AccessFlagBits::eShaderRead,
                                            vk::AccessFlagBits::eColorAttachmentWrite,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            imageViewCreateInfo.subresourceRange);
        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }

    CreateFramebuffer(cubemapPipeline, commandBuffer);
}

void PrefilteredCubemap::CreateFramebuffer(const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vk::ImageView attachment;
    vk::FramebufferCreateInfo frameBufferCI({}, cubemapPipeline.renderPass, 1, &attachment, imageSize_, imageSize_, 1);

    for (uint32_t i = 0; i < 6; i++) {
        attachment = cubemapFaceImageViews_[i];
        vkn::CheckResult(vkn::Device::GetBundle().device.createFramebuffer(&frameBufferCI, nullptr, &framebuffers_[i]));
    }
}

void PrefilteredCubemap::DrawPrefilteredCubemap(const Mesh& envCube, const vkn::Image& envMap, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    for (int mipLevel = 0; mipLevel < numMips_; mipLevel++) {
        int mipSize = 64 * std::pow(0.5, mipLevel);
        float roughness = (float)mipLevel / (float)(numMips_ - 1);

        Draw(mipSize, roughness, envCube, envMap, cubemapPipeline, commandBuffer);
        CopyToMipmap(mipSize, mipLevel, commandBuffer);
    }

    for (int i = 0; i < 6; i++) {
        vkn::Command::Begin(commandBuffer);
        imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            imageBundle_.image,
                                            vk::ImageLayout::eColorAttachmentOptimal,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            vk::AccessFlagBits::eColorAttachmentWrite,
                                            vk::AccessFlagBits::eShaderRead,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            imageViewCreateInfo.subresourceRange);
        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }

    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        mipmap.GetBundle().image,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eHostWrite,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        mipmap.imageViewCreateInfo.subresourceRange);
    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);
}

void PrefilteredCubemap::Draw(uint32_t mipSize, float roughness, const Mesh& envCube, const vkn::Image& envMap, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vkn::Command::Begin(commandBuffer);

    UpdateDescriptorSets(cubemapPipeline, envMap);

    vk::Viewport viewport({}, {}, (float)mipSize, (float)mipSize, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { imageSize_, imageSize_ });
    commandBuffer.setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        DrawPrefilteredCubemapFace(roughness, face, envCube, cubemapPipeline, commandBuffer);
    }

    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);

    for (int i = 0; i < 6; i++) {
        vkn::Command::Begin(commandBuffer);
        imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            imageBundle_.image,
                                            vk::ImageLayout::eColorAttachmentOptimal,
                                            vk::ImageLayout::eTransferSrcOptimal,
                                            vk::AccessFlagBits::eColorAttachmentWrite,
                                            vk::AccessFlagBits::eTransferRead,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            vk::PipelineStageFlagBits::eTransfer,
                                            imageViewCreateInfo.subresourceRange);
        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }
    imageBundle_.descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

void PrefilteredCubemap::CopyToMipmap(uint32_t mipSize, int mipLevel, vk::CommandBuffer& commandBuffer)
{
    for (int i = 0; i < 6; i++) {
        vkn::Command::Begin(commandBuffer);

        vk::ImageCopy copyRegion = {};

        copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        copyRegion.srcSubresource.baseArrayLayer = i;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;

        copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        copyRegion.dstSubresource.baseArrayLayer = i;
        copyRegion.dstSubresource.mipLevel = mipLevel;
        copyRegion.dstSubresource.layerCount = 1;

        copyRegion.extent.width = mipSize;
        copyRegion.extent.height = mipSize;
        copyRegion.extent.depth = 1;

        commandBuffer.copyImage(imageBundle_.image,
                                vk::ImageLayout::eTransferSrcOptimal,
                                mipmap.GetBundle().image,
                                vk::ImageLayout::eTransferDstOptimal,
                                1,
                                &copyRegion);

        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }

    for (int i = 0; i < 6; i++) {
        vkn::Command::Begin(commandBuffer);
        imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            imageBundle_.image,
                                            vk::ImageLayout::eTransferSrcOptimal,
                                            vk::ImageLayout::eColorAttachmentOptimal,
                                            vk::AccessFlagBits::eTransferRead,
                                            vk::AccessFlagBits::eColorAttachmentWrite,
                                            vk::PipelineStageFlagBits::eTransfer,
                                            vk::PipelineStageFlagBits::eAllCommands,
                                            imageViewCreateInfo.subresourceRange);
        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }
}

void PrefilteredCubemap::UpdateDescriptorSets(const vkn::Pipeline& cubemapPipeline, const vkn::Image& envMap)
{
    std::vector<vk::WriteDescriptorSet> writes = {
        { cubemapPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &envMap.GetBundle().descriptorImageInfo }
    };

    vkn::Device::GetBundle().device.updateDescriptorSets(writes, nullptr);
}

void PrefilteredCubemap::DrawPrefilteredCubemapFace(float roughness, uint32_t faceIndex, const Mesh& envCube, const vkn::Pipeline& cubemapPipeline, vk::CommandBuffer& commandBuffer)
{
    vk::ClearValue clearValues = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    vk::RenderPassBeginInfo renderPassBI(cubemapPipeline.renderPass, framebuffers_[faceIndex], { { 0, 0 }, { imageSize_, imageSize_ } }, 2, &clearValues);

    glm::mat4 viewMatrix;
    glm::mat4 projMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    switch (faceIndex) {
    case 0: // POS X
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 1: // NEG X
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 2: // POS Y
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    case 3: // NEG Y
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        break;
    case 4: // POS Z
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 5: // NEG Z
        viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    }

    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubemapPipeline.pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubemapPipeline.pipelineLayout, 0, 1, &cubemapPipeline.descriptorSets[0], 0, nullptr);

    vk::DeviceSize vertexOffsets[]{ 0 };
    pushConstants_.view = viewMatrix;
    pushConstants_.proj = projMatrix;
    pushConstants_.roughness = roughness;
    commandBuffer.pushConstants(
        cubemapPipeline.pipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(PrefilteredCubemapPushConstants),
        &pushConstants_);
    commandBuffer.bindVertexBuffers(0, 1, &envCube.vertexBuffers[0]->GetBundle().buffer, vertexOffsets);
    commandBuffer.bindIndexBuffer(envCube.indexBuffers[0]->GetBundle().buffer, 0, vk::IndexType::eUint32);
    commandBuffer.drawIndexed(envCube.GetIndicesCount(0), envCube.GetInstanceCount(), 0, 0, 0);

    commandBuffer.endRenderPass();
}

void PrefilteredCubemap::CreateMipmapSampler()
{
    vk::SamplerCreateInfo samplerCI;
    samplerCI.magFilter = vk::Filter::eLinear;
    samplerCI.minFilter = vk::Filter::eLinear;
    samplerCI.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerCI.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerCI.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerCI.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    samplerCI.minLod = 0.0f;
    samplerCI.maxLod = static_cast<float>(numMips_);
    samplerCI.borderColor = vk::BorderColor::eFloatOpaqueWhite;

    vkn::Device::GetBundle().device.createSampler(&samplerCI, nullptr, &mipmapSampler_);
}

PrefilteredCubemap::~PrefilteredCubemap()
{
    vkn::Device::GetBundle().device.destroySampler(mipmapSampler_);
}