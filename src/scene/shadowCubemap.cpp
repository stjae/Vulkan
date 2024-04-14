#include "shadowCubemap.h"

void ShadowCubemap::CreateShadowMap(vk::CommandBuffer& commandBuffer)
{
    CreateCubemap(shadowCubemapSize, shadowMapImageFormat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, commandBuffer);

    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        imageBundle_.image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eHostWrite,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        imageViewCreateInfo.subresourceRange);
    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);

    meshRenderPipeline.shadowCubeMapDescriptors.emplace_back(vkn::Image::repeatSampler, imageBundle_.imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
    CreateDepthImage(commandBuffer);
    CreateFramebuffer(commandBuffer);
}

void ShadowCubemap::CreateDepthImage(vk::CommandBuffer& commandBuffer)
{
    depthImage_.CreateImage({ shadowCubemapSize, shadowCubemapSize, 1 }, shadowMapDepthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);

    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        depthImage_.GetBundle().image,
                                        {},
                                        vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                        {},
                                        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);

    depthImage_.imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    depthImage_.CreateImageView();
}

void ShadowCubemap::CreateFramebuffer(vk::CommandBuffer& commandBuffer)
{
    std::array<vk::ImageView, 2> attachments;
    attachments[1] = depthImage_.GetBundle().imageView;

    vk::FramebufferCreateInfo frameBufferCI({}, shadowCubemapPipeline.renderPass, attachments.size(), attachments.data(), shadowCubemapSize, shadowCubemapSize, 1);

    for (uint32_t i = 0; i < 6; i++) {
        attachments[0] = cubemapFaceImageViews_[i];
        vkn::CheckResult(vkn::Device::GetBundle().device.createFramebuffer(&frameBufferCI, nullptr, &framebuffers_[i]));
    }
}

void ShadowCubemap::DrawShadowMap(vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<MeshModel>& meshes)
{
    vkn::Command::Begin(commandBuffer);

    vk::Viewport viewport({}, {}, (float)shadowCubemapSize, (float)shadowCubemapSize, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { shadowCubemapSize, shadowCubemapSize });
    commandBuffer.setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        UpdateCubemapFace(face, commandBuffer, lightIndex, lights, meshes);
    }

    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);
}

void ShadowCubemap::UpdateCubemapFace(uint32_t faceIndex, vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<MeshModel>& meshes)
{
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0] = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassBI(shadowCubemapPipeline.renderPass, framebuffers_[faceIndex], { { 0, 0 }, { shadowCubemapSize, shadowCubemapSize } }, 2, clearValues.data());

    glm::mat4 viewMatrix = lights[lightIndex].model;
    glm::vec3 lightPos = lights[lightIndex].model * glm::vec4(lights[lightIndex].pos, 1.0f);
    switch (faceIndex) {
    case 0: // POS X
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 1: // NEG X
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 2: // POS Y
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    case 3: // NEG Y
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        break;
    case 4: // POS Z
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 5: // NEG Z
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    }

    pushConstants_.view = viewMatrix;
    pushConstants_.lightIndex = lightIndex;

    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowCubemapPipeline.pipeline);
    int meshIndex = 0;
    vk::DeviceSize vertexOffsets[]{ 0 };
    for (auto& mesh : meshes) {
        if (mesh.GetInstanceCount() < 1)
            continue;
        pushConstants_.meshIndex = meshIndex;
        meshIndex++;
        commandBuffer.pushConstants(
            shadowCubemapPipeline.pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(ShadowMapPushConstants),
            &pushConstants_);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowCubemapPipeline.pipelineLayout, 0, 1, &shadowCubemapPipeline.descriptorSets[0], 0, nullptr);
        for (auto& part : mesh.GetMeshParts()) {
            commandBuffer.bindVertexBuffers(0, 1, &mesh.vertexBuffers[part.bufferIndex]->GetBundle().buffer, vertexOffsets);
            commandBuffer.bindIndexBuffer(mesh.indexBuffers[part.bufferIndex]->GetBundle().buffer, 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(mesh.GetIndicesCount(part.bufferIndex), mesh.GetInstanceCount(), 0, 0, 0);
        }
    }

    commandBuffer.endRenderPass();
}