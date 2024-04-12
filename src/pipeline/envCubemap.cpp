#include "envCubemap.h"

void EnvCubemapPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    shader.vertexShaderModule = vkn::Shader::CreateModule("shaders/environment.vert.spv");
    shader.fragmentShaderModule = vkn::Shader::CreateModule("shaders/environment.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(CubemapPushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, descriptorSetLayouts.size(), descriptorSetLayouts.data(), 1, &pushConstantRange);

    rasterizeStateCI_.cullMode = vk::CullModeFlagBits::eNone;

    pipelineCI_.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI_.pStages = shaderStageInfos.data();
    colorBlendStateCI_ = { {}, vk::False, {}, 1, &attachmentState };
    pipelineCI_.pColorBlendState = &colorBlendStateCI_;
    pipelineCI_.layout = vkn::Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfoCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (vkn::Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void EnvCubemapPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // hdr image
        { 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets);
    vkn::Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts);
}

void EnvCubemapPipeline::CreateRenderPass()
{
    vk::AttachmentDescription envCubemapAttachment;

    envCubemapAttachment.format = vk::Format::eR32G32B32A32Sfloat;
    envCubemapAttachment.samples = vk::SampleCountFlagBits::e1;
    envCubemapAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    envCubemapAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    envCubemapAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    envCubemapAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    envCubemapAttachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    envCubemapAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &envCubemapAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    renderPass = vkn::Device::GetBundle().device.createRenderPass(renderPassInfo);
}