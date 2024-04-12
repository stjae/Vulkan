#include "shadowCubemap.h"

void ShadowCubemapPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    shader.vertexShaderModule = vkn::Shader::CreateModule("shaders/pointlightshadow.vert.spv");
    shader.fragmentShaderModule = vkn::Shader::CreateModule("shaders/pointlightshadow.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShadowMapPushConstants) };
    vk::PipelineLayoutCreateInfo pipelineLayoutCI = { {}, (uint32_t)descriptorSetLayouts.size(), descriptorSetLayouts.data(), 1, &pushConstantRange };

    rasterizeStateCI_.cullMode = vk::CullModeFlagBits::eFront;
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

void ShadowCubemapPipeline::SetUpDescriptors()
{
    // Shadow Map
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // lights
        { 1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // meshes
        { 2, vk::DescriptorType::eStorageBuffer, 1000, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound }
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts);
}

void ShadowCubemapPipeline::CreateRenderPass()
{
    std::array<vk::AttachmentDescription, 2> shadowMapAttachments;

    shadowMapAttachments[0].format = shadowMapImageFormat;
    shadowMapAttachments[0].samples = vk::SampleCountFlagBits::e1;
    shadowMapAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    shadowMapAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    shadowMapAttachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    shadowMapAttachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    shadowMapAttachments[0].initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    shadowMapAttachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Depth
    shadowMapAttachments[1].format = shadowMapDepthFormat;
    shadowMapAttachments[1].samples = vk::SampleCountFlagBits::e1;
    shadowMapAttachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    shadowMapAttachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    shadowMapAttachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    shadowMapAttachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    shadowMapAttachments[1].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    shadowMapAttachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

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
    renderPassCI.attachmentCount = shadowMapAttachments.size();
    renderPassCI.pAttachments = shadowMapAttachments.data();
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDesc;

    vkn::CheckResult(vkn::Device::GetBundle().device.createRenderPass(&renderPassCI, nullptr, &renderPass));
}
