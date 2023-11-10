#include "pipeline.h"

void GraphicsPipeline::CreatePipeline()
{
    std::string vertexShaderFilepath = "shaders/shader.vert.spv";
    std::string fragmentShaderFilepath = "shaders/shader.frag.spv";

    vk::GraphicsPipelineCreateInfo pipelineInfo;

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

    // vertex input
    vk::VertexInputBindingDescription bindingDesc = scene->meshes[0]->GetBindingDesc();
    std::array<vk::VertexInputAttributeDescription, 2> attributeDescs = scene->meshes[0]->GetAttributeDescs();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    // input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

    // vertex shader
    shader.vertexShaderModule = shader.CreateModule(vertexShaderFilepath);
    Log(debug, fmt::terminal_color::bright_green, "created vertex shader module");
    vk::PipelineShaderStageCreateInfo vertexShaderInfo;
    vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertexShaderInfo.module = shader.vertexShaderModule;
    vertexShaderInfo.pName = "main";
    shaderStageInfos.push_back(vertexShaderInfo);

    std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data(), nullptr);
    pipelineInfo.pDynamicState = &dynamicStateInfo;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1;
    // viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    // viewportState.pScissors = &scissor;
    pipelineInfo.pViewportState = &viewportState;

    // rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = vk::CullModeFlagBits::eBack;
    rasterizerInfo.frontFace = vk::FrontFace::eClockwise;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    pipelineInfo.pRasterizationState = &rasterizerInfo;

    // fragment shader
    shader.fragmentShaderModule = shader.CreateModule(fragmentShaderFilepath);
    Log(debug, fmt::terminal_color::bright_green, "created fragment shader module");
    vk::PipelineShaderStageCreateInfo fragmentShaderInfo;
    fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragmentShaderInfo.module = shader.fragmentShaderModule;
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
    vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;
    colorBlendStateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments = &colorBlendAttachmentState;
    std::array<float, 4> blendConstants{ 0.0f, 0.0f, 0.0f, 0.0f };
    colorBlendStateInfo.blendConstants = blendConstants;
    pipelineInfo.pColorBlendState = &colorBlendStateInfo;

    // pipeline layout
    vkPipelineLayout = CreatePipelineLayout();
    pipelineInfo.layout = vkPipelineLayout;

    vkRenderPass = CreateRenderPass();
    pipelineInfo.renderPass = vkRenderPass;

    vkPipeline = (vkDevice.createGraphicsPipeline(nullptr, pipelineInfo)).value;
    Log(debug, fmt::terminal_color::bright_green, "created graphics pipeline");
}

vk::PipelineLayout GraphicsPipeline::CreatePipelineLayout()
{
    DescriptorSetLayoutData bindings;
    bindings.count = 2;
    bindings.indices.push_back(0);
    bindings.indices.push_back(1);
    bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
    bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
    bindings.counts.push_back(1);
    bindings.counts.push_back(1);
    bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    bindings.stages.push_back(vk::ShaderStageFlagBits::eFragment);

    descriptor.CreateSetLayout(bindings);
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptor.setLayout;

    return vkDevice.createPipelineLayout(pipelineLayoutInfo);
}

vk::RenderPass GraphicsPipeline::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.format = swapchainDetail.imageFormat;
    colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachmentDesc;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    return vkDevice.createRenderPass(renderPassInfo);
}

void GraphicsPipeline::CreateDescriptorPool()
{
    DescriptorSetLayoutData bindings;
    bindings.count = 1;
    bindings.types.push_back(vk::DescriptorType::eUniformBuffer);

    descriptor.CreatePool(static_cast<uint32_t>(swapchainDetail.frames.size()), bindings);
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDevice.destroyPipeline(vkPipeline);
    vkDevice.destroyPipelineLayout(vkPipelineLayout);
    vkDevice.destroyRenderPass(vkRenderPass);
    vkDevice.destroyDescriptorPool(descriptor.pool);
}