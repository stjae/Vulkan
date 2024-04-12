#include "meshRender.h"

void MeshRenderPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    shader.vertexShaderModule = vkn::Shader::CreateModule("shaders/base.vert.spv");
    shader.fragmentShaderModule = vkn::Shader::CreateModule("shaders/base.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentStates[0] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[1] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[1].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(MeshRenderPushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, descriptorSetLayouts.size(), descriptorSetLayouts.data(), 1, &pushConstantRange);

    pipelineCI_.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI_.pStages = shaderStageInfos.data();
    colorBlendStateCI_ = { {}, vk::False, {}, attachmentStates.size(), attachmentStates.data() };
    pipelineCI_.pColorBlendState = &colorBlendStateCI_;
    pipelineCI_.layout = vkn::Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfoCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (vkn::Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void MeshRenderPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // light
        { 1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // mesh
        { 2, vk::DescriptorType::eStorageBuffer, 1000, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    bindings = {
        // repeat sampler
        { 0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // albedo
        { 1, vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // normal
        { 2, vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // metallic
        { 3, vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // roughness
        { 4, vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // shadow cubemap
        { 5, vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    bindings = {
        // irradiance cubemap
        { 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
        // prefiltered cubemap
        { 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
        // brdfLUT
        { 2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts);
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

    renderPass = vkn::Device::GetBundle().device.createRenderPass(renderPassInfo);
}