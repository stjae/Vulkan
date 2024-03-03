#include "pipeline.h"

void Pipeline::CreateGraphicsPipeline(const vk::RenderPass& renderPass, const char* vertexShaderFilepath, const char* fragmentShaderFilepath, vk::PrimitiveTopology topology)
{
    vk::GraphicsPipelineCreateInfo pipelineInfo{};

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

    // vertex input
    vk::VertexInputBindingDescription bindingDesc = MeshData::GetBindingDesc();
    auto attributeDescs = MeshData::GetAttributeDescs();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescs.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    // input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.topology = topology;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

    // vertex shader
    shader_.vertexShaderModule = shader_.CreateModule(vertexShaderFilepath);
    Log(debug, fmt::terminal_color::bright_green, "created vertex shader module");
    vk::PipelineShaderStageCreateInfo vertexShaderInfo;
    vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertexShaderInfo.module = shader_.vertexShaderModule;
    vertexShaderInfo.pName = "main";
    shaderStageInfos.push_back(vertexShaderInfo);

    std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data(), nullptr);
    pipelineInfo.pDynamicState = &dynamicStateInfo;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    pipelineInfo.pViewportState = &viewportState;

    // rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = vk::CullModeFlagBits::eBack;
    rasterizerInfo.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    pipelineInfo.pRasterizationState = &rasterizerInfo;

    // fragment shader
    shader_.fragmentShaderModule = shader_.CreateModule(fragmentShaderFilepath);
    Log(debug, fmt::terminal_color::bright_green, "created fragment shader module");
    vk::PipelineShaderStageCreateInfo fragmentShaderInfo;
    fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragmentShaderInfo.module = shader_.fragmentShaderModule;
    fragmentShaderInfo.pName = "main";
    shaderStageInfos.push_back(fragmentShaderInfo);

    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
    pipelineInfo.pStages = shaderStageInfos.data();

    // multisampling
    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
    multisampleStateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
    pipelineInfo.pMultisampleState = &multisampleStateInfo;

    // color blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
    colorBlendAttachmentState.colorWriteMask = (vk::ColorComponentFlagBits::eR |
                                                vk::ColorComponentFlagBits::eG |
                                                vk::ColorComponentFlagBits::eB |
                                                vk::ColorComponentFlagBits::eA);
    colorBlendAttachmentState.blendEnable = VK_FALSE;

    vk::PipelineColorBlendAttachmentState idBlendAttachmentState;
    idBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR;
    idBlendAttachmentState.blendEnable = VK_FALSE;

    std::array<vk::PipelineColorBlendAttachmentState, 2> colorBlendAttachmentStates{ colorBlendAttachmentState, idBlendAttachmentState };
    vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;
    colorBlendStateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateInfo.attachmentCount = 2;
    colorBlendStateInfo.pAttachments = colorBlendAttachmentStates.data();
    std::array<float, 4> blendConstants{ 0.0f, 0.0f, 0.0f, 0.0f };
    colorBlendStateInfo.blendConstants = blendConstants;
    pipelineInfo.pColorBlendState = &colorBlendStateInfo;

    // depth stencil
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.depthTestEnable = vk::True;
    depthStencilInfo.depthWriteEnable = vk::True;
    depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
    depthStencilInfo.depthBoundsTestEnable = vk::False;
    depthStencilInfo.stencilTestEnable = vk::False;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;

    // pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts_.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts_.data();
    bundle_.pipelineLayout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfo);
    pipelineInfo.layout = bundle_.pipelineLayout;

    pipelineInfo.renderPass = renderPass;

    bundle_.pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
    Log(debug, fmt::terminal_color::bright_green, "created graphics pipeline");
}

void Pipeline::CreateMeshRenderDescriptorSetLayout()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<DescriptorBinding> bufferBindings;
    bufferBindings.emplace_back(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    bufferBindings.emplace_back(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    bufferBindings.emplace_back(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(bufferBindings));
    Descriptor::SetDescriptorPoolSize(poolSizes, bufferBindings, maxSets);

    std::vector<DescriptorBinding> combinedImageSamplerBindings;
    combinedImageSamplerBindings.emplace_back(0, vk::DescriptorType::eCombinedImageSampler, (int)Device::physicalDeviceLimits.maxDescriptorSetSamplers, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(combinedImageSamplerBindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    Descriptor::SetDescriptorPoolSize(poolSizes, combinedImageSamplerBindings, maxSets);

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
