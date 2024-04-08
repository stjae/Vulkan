#include "pipeline.h"
#include "device.h"
#include "descriptor.h"
#include "shader.h"

namespace vkn {

Pipeline::Pipeline()
{
    bindingDesc_ = MeshBase::GetBindingDesc();
    vertexInputAttribDesc_ = MeshBase::GetAttributeDescs();
    vertexInputStateCI_ = { {}, 1, &bindingDesc_, (uint32_t)vertexInputAttribDesc_.size(), vertexInputAttribDesc_.data() };
    inputAssemblyStateCI_ = { {}, vk::PrimitiveTopology::eTriangleList };
    dynamicStates_ = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    dynamicStateCI_ = { {}, (uint32_t)dynamicStates_.size(), dynamicStates_.data() };
    viewportStateCI_ = { {}, 1, {}, 1, {} };
    rasterizeStateCI_ = { {}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False, {}, {}, {}, 1.0f };
    multisampleStateCI_ = { {}, vk::SampleCountFlagBits::e1, vk::False };
    depthStencilStateCI_ = { {}, vk::True, vk::True, vk::CompareOp::eLess };

    pipelineCI_.pVertexInputState = &vertexInputStateCI_;
    pipelineCI_.pInputAssemblyState = &inputAssemblyStateCI_;
    pipelineCI_.pDynamicState = &dynamicStateCI_;
    pipelineCI_.pViewportState = &viewportStateCI_;
    pipelineCI_.pRasterizationState = &rasterizeStateCI_;
    pipelineCI_.pMultisampleState = &multisampleStateCI_;
    pipelineCI_.pDepthStencilState = &depthStencilStateCI_;
}

void MeshRenderPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    shader.vertexShaderModule = Shader::CreateModule("shaders/base.vert.spv");
    shader.fragmentShaderModule = Shader::CreateModule("shaders/base.frag.spv");
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
    pipelineCI_.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfoCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void ShadowMapPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    shader.vertexShaderModule = Shader::CreateModule("shaders/shadow.vert.spv");
    shader.fragmentShaderModule = Shader::CreateModule("shaders/shadow.frag.spv");
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
    pipelineCI_.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void EnvTexPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    shader.vertexShaderModule = Shader::CreateModule("shaders/genEnvTex.vert.spv");
    shader.fragmentShaderModule = Shader::CreateModule("shaders/genEnvTex.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GenEnvCubePushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, descriptorSetLayouts.size(), descriptorSetLayouts.data(), 1, &pushConstantRange);

    rasterizeStateCI_.cullMode = vk::CullModeFlagBits::eNone;

    pipelineCI_.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI_.pStages = shaderStageInfos.data();
    colorBlendStateCI_ = { {}, vk::False, {}, 1, &attachmentState };
    pipelineCI_.pColorBlendState = &colorBlendStateCI_;
    pipelineCI_.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfoCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void IrradianceCubemapPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    shader.vertexShaderModule = Shader::CreateModule("shaders/irradianceCubemap.vert.spv");
    shader.fragmentShaderModule = Shader::CreateModule("shaders/irradianceCubemap.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GenEnvCubePushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, descriptorSetLayouts.size(), descriptorSetLayouts.data(), 1, &pushConstantRange);

    rasterizeStateCI_.cullMode = vk::CullModeFlagBits::eNone;

    pipelineCI_.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI_.pStages = shaderStageInfos.data();
    colorBlendStateCI_ = { {}, vk::False, {}, 1, &attachmentState };
    pipelineCI_.pColorBlendState = &colorBlendStateCI_;
    pipelineCI_.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfoCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void SkyboxRenderPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    shader.vertexShaderModule = Shader::CreateModule("shaders/skybox.vert.spv");
    shader.fragmentShaderModule = Shader::CreateModule("shaders/skybox.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    attachmentStates[0] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[1] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[1].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, descriptorSetLayouts.size(), descriptorSetLayouts.data());

    depthStencilStateCI_.depthTestEnable = vk::False;
    depthStencilStateCI_.depthWriteEnable = vk::False;
    depthStencilStateCI_.depthCompareOp = vk::CompareOp::eLessOrEqual;
    rasterizeStateCI_.cullMode = vk::CullModeFlagBits::eFront;

    pipelineCI_.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI_.pStages = shaderStageInfos.data();
    colorBlendStateCI_ = { {}, vk::False, {}, attachmentStates.size(), attachmentStates.data() };
    pipelineCI_.pColorBlendState = &colorBlendStateCI_;
    pipelineCI_.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfoCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
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
        // cubemap
        { 5, vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts);
}

void ShadowMapPipeline::SetUpDescriptors()
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

void EnvTexPipeline::SetUpDescriptors()
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

void IrradianceCubemapPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // env cubemap
        { 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets);
    vkn::Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts);
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

void ShadowMapPipeline::CreateRenderPass()
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

void EnvTexPipeline::CreateRenderPass()
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

void IrradianceCubemapPipeline::CreateRenderPass()
{
    vk::AttachmentDescription attachment;

    attachment.format = vk::Format::eR32G32B32A32Sfloat;
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

void SkyboxRenderPipeline::CreateRenderPass()
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

Pipeline::~Pipeline()
{
    vkn::Device::GetBundle().device.destroyPipeline(pipeline);
    vkn::Device::GetBundle().device.destroyPipelineLayout(pipelineLayout);
    vkn::Device::GetBundle().device.destroyRenderPass(renderPass);
    for (auto& descriptorSetLayout : descriptorSetLayouts)
        vkn::Device::GetBundle().device.destroyDescriptorSetLayout(descriptorSetLayout);
    vkn::Device::GetBundle().device.destroyDescriptorPool(descriptorPool);
    if (shader.vertexShaderModule)
        vkn::Device::GetBundle().device.destroyShaderModule(shader.vertexShaderModule);
    if (shader.fragmentShaderModule)
        vkn::Device::GetBundle().device.destroyShaderModule(shader.fragmentShaderModule);
}

} // namespace vkn
