#include "pipeline.h"

void Pipeline::CreateMeshRenderPipeline(const vk::RenderPass& renderPass, const char* vertexShaderFilepath, const char* fragmentShaderFilepath)
{
    vk::GraphicsPipelineCreateInfo pipelineCI{};
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    shader_.vertexShaderModule = shader_.CreateModule(vertexShaderFilepath);
    shader_.fragmentShaderModule = shader_.CreateModule(fragmentShaderFilepath);
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader_.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader_.fragmentShaderModule, "main" };
    std::array<vk::DynamicState, 2> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    attachmentStates[0] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[1] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[1].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;

    vk::VertexInputBindingDescription bindingDesc = MeshData::GetBindingDesc();
    auto attributeDescs = MeshData::GetAttributeDescs();
    vk::PipelineVertexInputStateCreateInfo vertexInputStateCI({}, 1, &bindingDesc, attributeDescs.size(), attributeDescs.data());
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI({}, vk::PrimitiveTopology::eTriangleList);
    vk::PipelineDynamicStateCreateInfo dynamicStateCI({}, dynamicStates.size(), dynamicStates.data());
    vk::PipelineViewportStateCreateInfo viewportStateCI({}, 1, {}, 1, {});
    vk::PipelineRasterizationStateCreateInfo rasterizeStateCI({}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False);
    vk::PipelineMultisampleStateCreateInfo multisampleStateCI({}, vk::SampleCountFlagBits::e1, vk::False);
    vk::PipelineColorBlendStateCreateInfo colorBlendStateCI({}, vk::False, {}, attachmentStates.size(), attachmentStates.data());
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI({}, vk::True, vk::True, vk::CompareOp::eLess);
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, descriptorSetLayouts_.size(), descriptorSetLayouts_.data());

    pipelineCI.pVertexInputState = &vertexInputStateCI;
    pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI.pStages = shaderStageInfos.data();
    pipelineCI.pDynamicState = &dynamicStateCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pRasterizationState = &rasterizeStateCI;
    pipelineCI.pMultisampleState = &multisampleStateCI;
    pipelineCI.pColorBlendState = &colorBlendStateCI;
    pipelineCI.pDepthStencilState = &depthStencilStateCI;
    pipelineCI.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfo);
    pipelineCI.renderPass = renderPass;

    bundle_.pipelineLayout = pipelineCI.layout;
    bundle_.pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI)).value;
}

void Pipeline::CreateMeshRenderDescriptorSetLayout()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<DescriptorBinding> bufferBindings;
    // camera
    bufferBindings.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
    // lights
    bufferBindings.emplace_back(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eFragment);
    // meshes
    bufferBindings.emplace_back(2, vk::DescriptorType::eStorageBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(bufferBindings));
    Descriptor::SetDescriptorPoolSize(poolSizes, bufferBindings, maxSets);

    std::vector<DescriptorBinding> combinedImageSamplerBindings;
    // textures
    combinedImageSamplerBindings.emplace_back(0, vk::DescriptorType::eCombinedImageSampler, (int)Device::physicalDeviceLimits.maxDescriptorSetSamplers, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(combinedImageSamplerBindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    Descriptor::SetDescriptorPoolSize(poolSizes, combinedImageSamplerBindings, maxSets);

    Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts_);
}

void Pipeline::CreateShadowMapPipeline(const vk::RenderPass& renderPass, const char* vertexShaderFilepath, const char* fragmentShaderFilepath)
{
    vk::GraphicsPipelineCreateInfo pipelineCI{};
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachment;
    shader_.vertexShaderModule = shader_.CreateModule(vertexShaderFilepath);
    shader_.fragmentShaderModule = shader_.CreateModule(fragmentShaderFilepath);
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader_.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader_.fragmentShaderModule, "main" };
    std::array<vk::DynamicState, 2> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    attachment = vk::PipelineColorBlendAttachmentState(vk::False);
    attachment.colorWriteMask = vk::ColorComponentFlagBits::eR;
    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4));

    vk::VertexInputBindingDescription bindingDesc = MeshData::GetBindingDesc();
    auto attributeDescs = MeshData::GetAttributeDescs();
    vk::PipelineVertexInputStateCreateInfo vertexInputStateCI({}, 1, &bindingDesc, attributeDescs.size(), attributeDescs.data());
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI({}, vk::PrimitiveTopology::eTriangleList);
    vk::PipelineDynamicStateCreateInfo dynamicStateCI({}, dynamicStates.size(), dynamicStates.data());
    vk::PipelineViewportStateCreateInfo viewportStateCI({}, 1, {}, 1, {});
    vk::PipelineRasterizationStateCreateInfo rasterizeStateCI({}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False);
    vk::PipelineMultisampleStateCreateInfo multisampleStateCI({}, vk::SampleCountFlagBits::e1, vk::False);
    vk::PipelineColorBlendStateCreateInfo colorBlendStateCI({}, vk::False, {}, 1, &attachment);
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI({}, vk::True, vk::True, vk::CompareOp::eLess);
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, descriptorSetLayouts_.size(), descriptorSetLayouts_.data(), 1, &pushConstantRange);

    pipelineCI.pVertexInputState = &vertexInputStateCI;
    pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI.pStages = shaderStageInfos.data();
    pipelineCI.pDynamicState = &dynamicStateCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pRasterizationState = &rasterizeStateCI;
    pipelineCI.pMultisampleState = &multisampleStateCI;
    pipelineCI.pColorBlendState = &colorBlendStateCI;
    pipelineCI.pDepthStencilState = &depthStencilStateCI;
    pipelineCI.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfo);
    pipelineCI.renderPass = renderPass;

    bundle_.pipelineLayout = pipelineCI.layout;
    bundle_.pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI)).value;
}

void Pipeline::CreateShadowMapDescriptorSetLayout()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<DescriptorBinding> bufferBindings;
    // camera
    bufferBindings.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
    // lights
    bufferBindings.emplace_back(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex);
    // meshes
    bufferBindings.emplace_back(2, vk::DescriptorType::eStorageBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex);
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(bufferBindings));
    Descriptor::SetDescriptorPoolSize(poolSizes, bufferBindings, maxSets);

    Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts_);
}

Pipeline::~Pipeline()
{
    for (auto& layout : descriptorSetLayouts_)
        Device::GetBundle().device.destroyDescriptorSetLayout(layout);
    Device::GetBundle().device.destroyDescriptorPool(descriptorPool);
    Device::GetBundle().device.destroyPipeline(bundle_.pipeline);
    Device::GetBundle().device.destroyPipelineLayout(bundle_.pipelineLayout);
}
