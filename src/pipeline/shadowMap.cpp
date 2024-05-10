#include "shadowMap.h"

void ShadowMapPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    shader.vertexShaderModule = vkn::Shader::CreateModule("shader/dirlightshadow.vert.spv");
    shader.fragmentShaderModule = vkn::Shader::CreateModule("shader/dirlightshadow.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShadowMapPushConstants) };
    vk::PipelineLayoutCreateInfo pipelineLayoutCI = { {}, (uint32_t)descriptorSetLayouts.size(), descriptorSetLayouts.data(), 1, &pushConstantRange };

    rasterizeStateCI_.cullMode = vk::CullModeFlagBits::eNone;
    pipelineCI_.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI_.pStages = shaderStageInfos.data();
    pipelineCI_.renderPass = renderPass;
    colorBlendStateCI_ = { {}, vk::False, {}, 1, &attachmentState };
    pipelineCI_.pColorBlendState = &colorBlendStateCI_;
    pipelineCI_.layout = vkn::Device::GetBundle().device.createPipelineLayout(pipelineLayoutCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (vkn::Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void ShadowMapPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // meshes
        { 1, vk::DescriptorType::eStorageBuffer, 1000, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound }
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts);
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

    vkn::CheckResult(vkn::Device::GetBundle().device.createRenderPass(&renderPassCI, nullptr, &renderPass));
}

void ShadowMapPipeline::UpdateShadowMapSpaceViewProjDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSets[0];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &shadowMapSpaceViewProjDescriptor;
    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void ShadowMapPipeline::UpdateMeshDescriptors()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSets[0];
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = meshDescriptors.size();
    writeDescriptorSet.descriptorType = vk::DescriptorType::eStorageBuffer;
    writeDescriptorSet.pBufferInfo = meshDescriptors.data();
    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}
