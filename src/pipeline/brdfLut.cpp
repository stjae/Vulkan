#include "brdfLut.h"

void BrdfLutPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    shader.vertexShaderModule = vkn::Shader::CreateModule("shaders/brdflut.vert.spv");
    shader.fragmentShaderModule = vkn::Shader::CreateModule("shaders/brdflut.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    CreateRenderPass();

    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI;

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

void BrdfLutPipeline::CreateRenderPass()
{
    vk::AttachmentDescription attachment;

    attachment.format = vk::Format::eR16G16Sfloat;
    attachment.samples = vk::SampleCountFlagBits::e1;
    attachment.loadOp = vk::AttachmentLoadOp::eClear;
    attachment.storeOp = vk::AttachmentStoreOp::eStore;
    attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    renderPass = vkn::Device::GetBundle().device.createRenderPass(renderPassInfo);
}
