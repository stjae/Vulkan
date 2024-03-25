#include "shadowMap.h"

void ShadowMap::PrepareShadowCubeMap(vk::CommandBuffer& commandBuffer)
{
    vk::ImageCreateInfo shadowCubeMapImageCI(vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D, shadowMapImageFormat, { shadowMapSize, shadowMapSize, 1 }, 1, 6, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive);
    shadowCubeMap.CreateImage(shadowCubeMapImageCI, vk::MemoryPropertyFlagBits::eDeviceLocal);
    Command::Begin(commandBuffer);
    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);
    Command::SetImageMemoryBarrier(commandBuffer,
                                   shadowCubeMap,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eHostWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eAllCommands,
                                   vk::PipelineStageFlagBits::eAllCommands,
                                   subresourceRange);
    commandBuffer.end();
    Command::Submit(&commandBuffer, 1);

    shadowCubeMap.CreateSampler();

    vk::ImageViewCreateInfo viewCI({}, shadowCubeMap.GetBundle().image, vk::ImageViewType::eCube, shadowMapImageFormat, { vk::ComponentSwizzle::eR }, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6 });
    shadowCubeMap.CreateImageView(viewCI);
    shadowCubeMap.SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
    meshRenderPipeline.shadowCubeMapDescriptors.push_back(shadowCubeMap.GetBundle().imageInfo);

    viewCI.viewType = vk::ImageViewType::e2D;
    viewCI.subresourceRange.layerCount = 1;

    for (uint32_t i = 0; i < 6; i++) {
        viewCI.subresourceRange.baseArrayLayer = i;
        Device::GetBundle().device.createImageView(&viewCI, nullptr, &shadowCubeMapFaceImageViews[i]);
    }

    CreateFrameBuffer(commandBuffer);
}

void ShadowMap::DrawShadowMap(vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<Mesh>& meshes)
{
    Command::Begin(commandBuffer);

    vk::Viewport viewport({}, {}, (float)shadowMapSize, (float)shadowMapSize, 0.0f, 1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor({ 0, 0 }, { shadowMapSize, shadowMapSize });
    commandBuffer.setScissor(0, 1, &scissor);

    for (uint32_t face = 0; face < 6; face++) {
        UpdateCubeMapFace(face, commandBuffer, lightIndex, lights, meshes);
    }

    commandBuffer.end();
    Command::Submit(&commandBuffer, 1);
}

void ShadowMap::UpdateCubeMapFace(uint32_t faceIndex, vk::CommandBuffer& commandBuffer, int lightIndex, std::vector<LightData>& lights, std::vector<Mesh>& meshes)
{
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0] = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    vk::RenderPassBeginInfo renderPassBI(shadowMapPipeline.renderPass, framebuffers[faceIndex], { { 0, 0 }, { shadowMapSize, shadowMapSize } }, 2, clearValues.data());

    glm::mat4 viewMatrix = lights[lightIndex].model;
    glm::vec3 lightPos = lights[lightIndex].model * glm::vec4(lights[lightIndex].pos, 1.0f);
    switch (faceIndex) {
    case 0: // POSITIVE_X
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 1: // NEGATIVE_X
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 2: // POSITIVE_Y
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    case 3: // NEGATIVE_Y
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        break;
    case 4: // POSITIVE_Z
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    case 5: // NEGATIVE_Z
        viewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        break;
    }

    shadowMapPushConsts.view = viewMatrix;
    shadowMapPushConsts.lightIndex = lightIndex;

    commandBuffer.beginRenderPass(&renderPassBI, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadowMapPipeline.pipeline);
    int meshIndex = 0;
    vk::DeviceSize vertexOffsets[]{ 0 };
    for (auto& mesh : meshes) {
        if (mesh.GetInstanceCount() < 1)
            continue;
        shadowMapPushConsts.meshIndex = meshIndex;
        meshIndex++;
        commandBuffer.pushConstants(
            shadowMapPipeline.pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(ShadowMapPushConstants),
            &shadowMapPushConsts);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadowMapPipeline.pipelineLayout, 0, 1, &shadowMapPipeline.descriptorSets[0], 0, nullptr);
        for (auto& part : mesh.GetMeshParts()) {
            commandBuffer.bindVertexBuffers(0, 1, &mesh.vertexBuffers[part.bufferIndex]->GetBundle().buffer, vertexOffsets);
            commandBuffer.bindIndexBuffer(mesh.indexBuffers[part.bufferIndex]->GetBundle().buffer, 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(mesh.GetIndicesCount(part.bufferIndex), mesh.GetInstanceCount(), 0, 0, 0);
        }
    }

    commandBuffer.endRenderPass();
}
void ShadowMap::CreateFrameBuffer(vk::CommandBuffer& commandBuffer)
{
    vk::ImageCreateInfo imageCI({}, vk::ImageType::e2D, shadowMapDepthFormat, { shadowMapSize, shadowMapSize, 1 }, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, {}, {}, vk::ImageLayout::eUndefined);
    depth.CreateImage(imageCI, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::ImageViewCreateInfo depthStencilViewCI({}, depth.GetBundle().image, vk::ImageViewType::e2D, shadowMapDepthFormat, {}, { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

    Command::Begin(commandBuffer);
    Command::SetImageMemoryBarrier(commandBuffer, depth, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, {}, vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });
    commandBuffer.end();
    Command::Submit(&commandBuffer, 1);

    depth.CreateImageView(depthStencilViewCI);

    std::array<vk::ImageView, 2> attachments;
    attachments[1] = depth.GetBundle().imageView;

    vk::FramebufferCreateInfo frameBufferCI({}, shadowMapPipeline.renderPass, attachments.size(), attachments.data(), shadowMapSize, shadowMapSize, 1);

    for (uint32_t i = 0; i < 6; i++) {
        attachments[0] = shadowCubeMapFaceImageViews[i];
        Device::GetBundle().device.createFramebuffer(&frameBufferCI, nullptr, &framebuffers[i]);
    }
}

ShadowMap::~ShadowMap()
{
    for (auto& framebuffer : framebuffers)
        Device::GetBundle().device.destroyFramebuffer(framebuffer);
    for (auto& imageView : shadowCubeMapFaceImageViews)
        Device::GetBundle().device.destroyImageView(imageView);
}
