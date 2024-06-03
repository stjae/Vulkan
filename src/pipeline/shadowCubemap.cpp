#include "shadowCubemap.h"

void ShadowCubemapPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    m_shaderModule.m_vertexShaderModule = vkn::Shader::CreateModule("shader/pointlightshadow.vert.spv");
    m_shaderModule.m_fragmentShaderModule = vkn::Shader::CreateModule("shader/pointlightshadow.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, m_shaderModule.m_vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, m_shaderModule.m_fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShadowCubemapPushConstants) };
    vk::PipelineLayoutCreateInfo pipelineLayoutCI = { {}, (uint32_t)m_descriptorSetLayouts.size(), m_descriptorSetLayouts.data(), 1, &pushConstantRange };

    m_rasterizeStateCI.cullMode = vk::CullModeFlagBits::eFront;
    m_pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    m_pipelineCI.pStages = shaderStageInfos.data();
    m_pipelineCI.renderPass = m_renderPass;
    m_colorBlendStateCI = { {}, vk::False, {}, 1, &attachmentState };
    m_pipelineCI.pColorBlendState = &m_colorBlendStateCI;
    m_pipelineCI.layout = vkn::Device::Get().device.createPipelineLayout(pipelineLayoutCI);
    m_pipelineCI.renderPass = m_renderPass;

    m_pipelineLayout = m_pipelineCI.layout;
    m_pipeline = (vkn::Device::Get().device.createGraphicsPipeline(nullptr, m_pipelineCI)).value;
}

void ShadowCubemapPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // lights
        { vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // meshes
        { vk::DescriptorType::eStorageBuffer, 1000, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound }
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(m_descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(m_descriptorPool, m_descriptorSets, m_descriptorSetLayouts);
}

void ShadowCubemapPipeline::CreateRenderPass()
{
    std::array<vk::AttachmentDescription, 2> attachments;

    attachments[0].format = shadowMapImageFormat;
    attachments[0].samples = vk::SampleCountFlagBits::e1;
    attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Depth
    attachments[1].format = shadowMapDepthFormat;
    attachments[1].samples = vk::SampleCountFlagBits::e1;
    attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    vk::RenderPassCreateInfo renderPassCI{};
    renderPassCI.attachmentCount = attachments.size();
    renderPassCI.pAttachments = attachments.data();
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDesc;

    vkn::CheckResult(vkn::Device::Get().device.createRenderPass(&renderPassCI, nullptr, &m_renderPass));
}

void ShadowCubemapPipeline::UpdateProjBuffer(const vk::DescriptorBufferInfo& bufferInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo), nullptr);
}

void ShadowCubemapPipeline::UpdatePointLightUBO(const vk::DescriptorBufferInfo& bufferInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &bufferInfo), nullptr);
}

void ShadowCubemapPipeline::UpdateMeshUBO(const std::vector<vk::DescriptorBufferInfo>& bufferInfos)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 2, 0, bufferInfos.size(), vk::DescriptorType::eStorageBuffer, nullptr, bufferInfos.data()), nullptr);
}
