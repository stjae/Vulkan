#include "meshRender.h"

void MeshRenderPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    m_shaderModule.m_vertexShaderModule = vkn::Shader::CreateModule("shader/base.vert.spv");
    m_shaderModule.m_fragmentShaderModule = vkn::Shader::CreateModule("shader/base.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, m_shaderModule.m_vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, m_shaderModule.m_fragmentShaderModule, "main" };
    attachmentStates[0] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[1] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[1].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(MeshRenderPushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, m_descriptorSetLayouts.size(), m_descriptorSetLayouts.data(), 1, &pushConstantRange);

    m_pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    m_pipelineCI.pStages = shaderStageInfos.data();
    m_colorBlendStateCI = { {}, vk::False, {}, attachmentStates.size(), attachmentStates.data() };
    m_pipelineCI.pColorBlendState = &m_colorBlendStateCI;
    m_pipelineCI.layout = vkn::Device::Get().device.createPipelineLayout(pipelineLayoutInfoCI);
    m_pipelineCI.renderPass = m_renderPass;

    m_pipelineLayout = m_pipelineCI.layout;
    m_pipeline = (vkn::Device::Get().device.createGraphicsPipeline(nullptr, m_pipelineCI)).value;
}

void MeshRenderPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // light
        { vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // mesh
        { vk::DescriptorType::eStorageBuffer, 1000, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    bindings = {
        // repeat sampler
        { vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // albedo
        { vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // normal
        { vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // metallic
        { vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // roughness
        { vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // shadow cubemap
        { vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    bindings = {
        // irradiance cubemap
        { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
        // prefiltered cubemap
        { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
        // brdfLUT
        { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
        // shadow map
        { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    bindings = {
        // shadowMap view projection
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
        // directional light
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment },
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(m_descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(m_descriptorPool, m_descriptorSets, m_descriptorSetLayouts);
}

void MeshRenderPipeline::CreateRenderPass()
{
    std::array<vk::AttachmentDescription, 3> meshRenderAttachments;

    meshRenderAttachments[0].format = vk::Format::eB8G8R8A8Srgb;
    meshRenderAttachments[0].samples = vk::SampleCountFlagBits::e1;
    meshRenderAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    meshRenderAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    meshRenderAttachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    meshRenderAttachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    meshRenderAttachments[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    meshRenderAttachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // ID
    meshRenderAttachments[1].format = vk::Format::eR32G32Sint;
    meshRenderAttachments[1].samples = vk::SampleCountFlagBits::e1;
    meshRenderAttachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    meshRenderAttachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    meshRenderAttachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    meshRenderAttachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    meshRenderAttachments[1].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    meshRenderAttachments[1].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // Depth
    meshRenderAttachments[2].format = vk::Format::eD32Sfloat;
    meshRenderAttachments[2].samples = vk::SampleCountFlagBits::e1;
    meshRenderAttachments[2].loadOp = vk::AttachmentLoadOp::eClear;
    meshRenderAttachments[2].storeOp = vk::AttachmentStoreOp::eStore;
    meshRenderAttachments[2].stencilLoadOp = vk::AttachmentLoadOp::eClear;
    meshRenderAttachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    meshRenderAttachments[2].initialLayout = vk::ImageLayout::eUndefined;
    meshRenderAttachments[2].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference idAttachmentRef;
    idAttachmentRef.attachment = 1;
    idAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    std::array<vk::AttachmentReference, 2> colorAttachmentRefs{ colorAttachmentRef, idAttachmentRef };

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = colorAttachmentRefs.size();
    subpassDesc.pColorAttachments = colorAttachmentRefs.data();
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = (uint32_t)(meshRenderAttachments.size());
    renderPassInfo.pAttachments = meshRenderAttachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    m_renderPass = vkn::Device::Get().device.createRenderPass(renderPassInfo);
}

void MeshRenderPipeline::UpdateCameraDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[0];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &m_cameraDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void MeshRenderPipeline::UpdatePointLightDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[0];
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eStorageBuffer;
    writeDescriptorSet.pBufferInfo = &m_pointLightDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void MeshRenderPipeline::UpdateMeshDescriptors()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[0];
    writeDescriptorSet.dstBinding = 2;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = m_meshDescriptors.size();
    writeDescriptorSet.descriptorType = vk::DescriptorType::eStorageBuffer;
    writeDescriptorSet.pBufferInfo = m_meshDescriptors.data();
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void MeshRenderPipeline::UpdateShadowMapDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[2];
    writeDescriptorSet.dstBinding = 3;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.pImageInfo = &m_shadowMapImageDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void MeshRenderPipeline::UpdateShadowCubemapDescriptors()
{
    if (!m_shadowCubemapDescriptors.empty()) {
        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = m_descriptorSets[1];
        writeDescriptorSet.dstBinding = 5;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorCount = m_shadowCubemapDescriptors.size();
        writeDescriptorSet.descriptorType = vk::DescriptorType::eSampledImage;
        writeDescriptorSet.pImageInfo = m_shadowCubemapDescriptors.data();
        vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }
}

void MeshRenderPipeline::UpdateShadowMapSpaceViewProjDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[3];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &m_shadowMapSpaceViewProjDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void MeshRenderPipeline::UpdateAlbedoTextureWriteDescriptors()
{
    if (!m_albeodoTextureDescriptors.empty()) {
        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = m_descriptorSets[1];
        writeDescriptorSet.dstBinding = 1;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorCount = m_albeodoTextureDescriptors.size();
        writeDescriptorSet.descriptorType = vk::DescriptorType::eSampledImage;
        writeDescriptorSet.pImageInfo = m_albeodoTextureDescriptors.data();
        vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }
}

void MeshRenderPipeline::UpdateNormalTextureWriteDescriptors()
{
    if (!m_normalTextureDescriptors.empty()) {
        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = m_descriptorSets[1];
        writeDescriptorSet.dstBinding = 2;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorCount = m_normalTextureDescriptors.size();
        writeDescriptorSet.descriptorType = vk::DescriptorType::eSampledImage;
        writeDescriptorSet.pImageInfo = m_normalTextureDescriptors.data();
        vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }
}

void MeshRenderPipeline::UpdateMetallicTextureWriteDescriptors()
{
    if (!m_metallicTextureDescriptors.empty()) {
        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = m_descriptorSets[1];
        writeDescriptorSet.dstBinding = 3;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorCount = m_metallicTextureDescriptors.size();
        writeDescriptorSet.descriptorType = vk::DescriptorType::eSampledImage;
        writeDescriptorSet.pImageInfo = m_normalTextureDescriptors.data();
        vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }
}

void MeshRenderPipeline::UpdateRoughnessTextureWriteDescriptors()
{
    if (!m_roughnessTextureDescriptors.empty()) {
        vk::WriteDescriptorSet writeDescriptorSet;
        writeDescriptorSet.dstSet = m_descriptorSets[1];
        writeDescriptorSet.dstBinding = 4;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorCount = m_roughnessTextureDescriptors.size();
        writeDescriptorSet.descriptorType = vk::DescriptorType::eSampledImage;
        writeDescriptorSet.pImageInfo = m_roughnessTextureDescriptors.data();
        vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }
}

void MeshRenderPipeline::UpdateIrraianceCubemapDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[2];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.pImageInfo = &m_irradianceCubemapDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void MeshRenderPipeline::UpdatePrefilteredCubemapDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[2];
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.pImageInfo = &m_prefilteredCubemapDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void MeshRenderPipeline::UpdateBrdfLutDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[2];
    writeDescriptorSet.dstBinding = 2;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.pImageInfo = &m_brdfLutDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void MeshRenderPipeline::UpdateDirLightDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[3];
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &m_dirLightDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}
