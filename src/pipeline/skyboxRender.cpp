#include "skyboxRender.h"

void SkyboxRenderPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    shader.vertexShaderModule = vkn::Shader::CreateModule("shaders/skybox.vert.spv");
    shader.fragmentShaderModule = vkn::Shader::CreateModule("shaders/skybox.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentStates[0] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[1] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[1].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eFragment, 0, sizeof(SkyboxRenderPushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, descriptorSetLayouts.size(), descriptorSetLayouts.data(), 1, &pushConstantRange);

    depthStencilStateCI_.depthTestEnable = vk::False;
    depthStencilStateCI_.depthWriteEnable = vk::False;
    depthStencilStateCI_.depthCompareOp = vk::CompareOp::eLessOrEqual;
    rasterizeStateCI_.cullMode = vk::CullModeFlagBits::eFront;

    pipelineCI_.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI_.pStages = shaderStageInfos.data();
    colorBlendStateCI_ = { {}, vk::False, {}, attachmentStates.size(), attachmentStates.data() };
    pipelineCI_.pColorBlendState = &colorBlendStateCI_;
    pipelineCI_.layout = vkn::Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfoCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (vkn::Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void SkyboxRenderPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
        // hdr image
        { 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets);
    vkn::Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts);
}

void SkyboxRenderPipeline::CreateRenderPass()
{
    std::array<vk::AttachmentDescription, 3> attachments;

    attachments[0].format = vk::Format::eB8G8R8A8Srgb;
    attachments[0].samples = vk::SampleCountFlagBits::e1;
    attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // ID
    attachments[1].format = vk::Format::eR32G32Sint;
    attachments[1].samples = vk::SampleCountFlagBits::e1;
    attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    attachments[1].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // Depth
    attachments[2].format = vk::Format::eD32Sfloat;
    attachments[2].samples = vk::SampleCountFlagBits::e1;
    attachments[2].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[2].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[2].stencilLoadOp = vk::AttachmentLoadOp::eClear;
    attachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[2].initialLayout = vk::ImageLayout::eUndefined;
    attachments[2].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

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
    renderPassInfo.attachmentCount = (uint32_t)(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    renderPass = vkn::Device::GetBundle().device.createRenderPass(renderPassInfo);
}

void SkyboxRenderPipeline::UpdateCameraDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSets[0];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &cameraDescriptor;
    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void SkyboxRenderPipeline::UpdateIrradianceCubemapDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSets[0];
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    writeDescriptorSet.pImageInfo = &irradianceCubemapDescriptor;
    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}
