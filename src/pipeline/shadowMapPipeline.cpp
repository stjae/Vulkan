#include "shadowMapPipeline.h"

ShadowMapPipeline shadowMapPipeline;

void ShadowMapPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    m_shaderModule.m_vertexShaderModule = vkn::Shader::CreateModule("shader/spv/dirlightshadow.vert.spv");
    m_shaderModule.m_fragmentShaderModule = vkn::Shader::CreateModule("shader/spv/dirlightshadow.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, m_shaderModule.m_vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, m_shaderModule.m_fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShadowMapPushConstants) };
    vk::PipelineLayoutCreateInfo pipelineLayoutCI = { {}, (uint32_t)m_descriptorSetLayouts.size(), m_descriptorSetLayouts.data(), 1, &pushConstantRange };

    m_rasterizeStateCI.cullMode = vk::CullModeFlagBits::eNone;
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

void ShadowMapPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // meshes
        { vk::DescriptorType::eStorageBuffer, 1000, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound }
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(m_descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(m_descriptorPool, m_descriptorSets, m_descriptorSetLayouts);
}

void ShadowMapPipeline::CreateRenderPass()
{
    vk::AttachmentDescription attachment;

    // Depth
    attachment.format = shadowMapDepthFormat;
    attachment.samples = vk::SampleCountFlagBits::e1;
    attachment.loadOp = vk::AttachmentLoadOp::eClear;
    attachment.storeOp = vk::AttachmentStoreOp::eStore;
    attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachment.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 0;
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    vk::RenderPassCreateInfo renderPassCI{};
    renderPassCI.attachmentCount = 1;
    renderPassCI.pAttachments = &attachment;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDesc;

    vkn::CHECK_RESULT(vkn::Device::Get().device.createRenderPass(&renderPassCI, nullptr, &m_renderPass));
}

void ShadowMapPipeline::UpdateViewProjBuffer(const vk::DescriptorBufferInfo& bufferInfo)
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[0];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void ShadowMapPipeline::UpdateMeshUBOBuffer(const std::vector<vk::DescriptorBufferInfo>& bufferInfos)
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[0];
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = bufferInfos.size();
    writeDescriptorSet.descriptorType = vk::DescriptorType::eStorageBuffer;
    writeDescriptorSet.pBufferInfo = bufferInfos.data();
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}
