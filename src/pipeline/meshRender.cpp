#include "meshRender.h"

void MeshRenderPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    m_shaderModule.m_vertexShaderModule = vkn::Shader::CreateModule("shader/spv/base.vert.spv");
    m_shaderModule.m_fragmentShaderModule = vkn::Shader::CreateModule("shader/spv/base.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, m_shaderModule.m_vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, m_shaderModule.m_fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    m_clearValues = { vk::ClearValue({ 0.05f, 0.05f, 0.05f, 1.0f }),
                      vk::ClearValue({ 0.05f, 0.05f, 0.05f, 1.0f }),
                      vk::ClearValue(vk::ClearDepthStencilValue(1.0f)),
                      vk::ClearValue({ 0.05f, 0.05f, 0.05f, 1.0f }) };

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(MeshRenderPushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, m_descriptorSetLayouts.size(), m_descriptorSetLayouts.data(), 1, &pushConstantRange);

    m_pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    m_pipelineCI.pStages = shaderStageInfos.data();
    m_colorBlendStateCI = { {}, vk::False, {}, 1, &attachmentState };
    m_pipelineCI.pColorBlendState = &m_colorBlendStateCI;
    m_pipelineCI.layout = vkn::Device::Get().device.createPipelineLayout(pipelineLayoutInfoCI);
    m_pipelineCI.renderPass = m_renderPass;
    m_multisampleStateCI.rasterizationSamples = vkn::Device::Get().maxSampleCount;
    m_pipelineCI.subpass = 0;

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
        // cascade
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
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
        // cascade
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment },
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(m_descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(m_descriptorPool, m_descriptorSets, m_descriptorSetLayouts);
}

void MeshRenderPipeline::CreateRenderPass()
{
    std::array<vk::AttachmentDescription, 4> attachments;

    // Color
    attachments[0].format = vk::Format::eB8G8R8A8Srgb;
    attachments[0].samples = vkn::Device::Get().maxSampleCount;
    attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].initialLayout = vk::ImageLayout::eUndefined;
    attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Color Resolve
    attachments[1].format = vk::Format::eB8G8R8A8Srgb;
    attachments[1].samples = vk::SampleCountFlagBits::e1;
    attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].initialLayout = vk::ImageLayout::eUndefined;
    attachments[1].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Depth
    attachments[2].format = vk::Format::eD32Sfloat;
    attachments[2].samples = vkn::Device::Get().maxSampleCount;
    attachments[2].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[2].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[2].stencilLoadOp = vk::AttachmentLoadOp::eClear;
    attachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[2].initialLayout = vk::ImageLayout::eUndefined;
    attachments[2].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    // Post Process
    attachments[3].format = vk::Format::eB8G8R8A8Srgb;
    attachments[3].samples = vk::SampleCountFlagBits::e1;
    attachments[3].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[3].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[3].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[3].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[3].initialLayout = vk::ImageLayout::eUndefined;
    attachments[3].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentResolveRef;
    colorAttachmentResolveRef.attachment = 1;
    colorAttachmentResolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference postProcessAttachmentRef;
    postProcessAttachmentRef.attachment = 3;
    postProcessAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;
    subpassDesc.pResolveAttachments = &colorAttachmentResolveRef;

    // post process
    vk::SubpassDescription postProcessSubpassDesc;
    postProcessSubpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    postProcessSubpassDesc.colorAttachmentCount = 1;
    postProcessSubpassDesc.pColorAttachments = &postProcessAttachmentRef;

    std::array<vk::SubpassDescription, 2> subpasses = { subpassDesc, postProcessSubpassDesc };

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();

    m_renderPass = vkn::Device::Get().device.createRenderPass(renderPassInfo);
}

void MeshRenderPipeline::UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo), nullptr);
}

void MeshRenderPipeline::UpdatePointLightUBO(const vk::DescriptorBufferInfo& bufferInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &bufferInfo), nullptr);
}

void MeshRenderPipeline::UpdateMeshUBO(const std::vector<vk::DescriptorBufferInfo>& bufferInfos)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 2, 0, bufferInfos.size(), vk::DescriptorType::eStorageBuffer, nullptr, bufferInfos.data()), nullptr);
}

void MeshRenderPipeline::UpdateSampler(const vk::DescriptorImageInfo& imageInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[1], 0, 0, 1, vk::DescriptorType::eSampler, &imageInfo), nullptr);
}

void MeshRenderPipeline::UpdateAlbedoTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos)
{
    if (!imageInfos.empty()) {
        vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[1], 1, 0, imageInfos.size(), vk::DescriptorType::eSampledImage, imageInfos.data()), nullptr);
    }
}

void MeshRenderPipeline::UpdateNormalTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos)
{
    if (!imageInfos.empty()) {
        vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[1], 2, 0, imageInfos.size(), vk::DescriptorType::eSampledImage, imageInfos.data()), nullptr);
    }
}

void MeshRenderPipeline::UpdateMetallicTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos)
{
    if (!imageInfos.empty()) {
        vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[1], 3, 0, imageInfos.size(), vk::DescriptorType::eSampledImage, imageInfos.data()), nullptr);
    }
}

void MeshRenderPipeline::UpdateRoughnessTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos)
{
    if (!imageInfos.empty()) {
        vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[1], 4, 0, imageInfos.size(), vk::DescriptorType::eSampledImage, imageInfos.data()), nullptr);
    }
}

void MeshRenderPipeline::UpdateShadowCubemap(const std::vector<vk::DescriptorImageInfo>& imageInfos)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[1], 5, 0, imageInfos.size(), vk::DescriptorType::eSampledImage, imageInfos.data()), nullptr);
}

void MeshRenderPipeline::UpdateIrraianceCubemap(const vk::DescriptorImageInfo& imageInfo)
{
    vk::WriteDescriptorSet writeDescriptorSet;
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[2], 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo), nullptr);
}

void MeshRenderPipeline::UpdatePrefilteredCubemap(const vk::DescriptorImageInfo& imageInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[2], 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo), nullptr);
}

void MeshRenderPipeline::UpdateBrdfLut(const vk::DescriptorImageInfo& imageInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[2], 2, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo), nullptr);
}

void MeshRenderPipeline::UpdateShadowMap(const vk::DescriptorImageInfo& imageInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[2], 3, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo), nullptr);
}

void MeshRenderPipeline::UpdateCascadeUBO(const vk::DescriptorBufferInfo& bufferInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[3], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo), nullptr);
}
