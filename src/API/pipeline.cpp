#include "pipeline.h"

void GraphicsPipeline::CreatePipeline()
{
    std::string vertexShaderFilepath = "shaders/shader.vert.spv";
    std::string fragmentShaderFilepath = "shaders/shader.frag.spv";

    vk::GraphicsPipelineCreateInfo pipelineInfo;

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

    // vertex input
    // vk::VertexInputBindingDescription bindingDesc = GetPosColorBindingDesc();
    // std::array<vk::VertexInputAttributeDescription, 2> attributeDescs = GetPosColorAttributeDescs();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    // vertexInputInfo.setVertexBindingDescriptionCount(1);
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    // vertexInputInfo.setPVertexBindingDescriptions(&bindingDesc);
    // vertexInputInfo.setVertexAttributeDescriptionCount(2);
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    // vertexInputInfo.setPVertexAttributeDescriptions(attributeDescs.data());
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    // input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

    // vertex shader
    m_shader.vertexShaderModule = m_shader.CreateModule(vertexShaderFilepath);
    Log(debug, fmt::terminal_color::bright_green, "created vertex shader module");
    vk::PipelineShaderStageCreateInfo vertexShaderInfo;
    vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertexShaderInfo.module = m_shader.vertexShaderModule;
    vertexShaderInfo.pName = "main";
    shaderStageInfos.push_back(vertexShaderInfo);

    // viewport and scissor
    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainDetails.extent.width);
    viewport.height = static_cast<float>(swapchainDetails.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = swapchainDetails.extent;

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
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
    m_shader.fragmentShaderModule = m_shader.CreateModule(fragmentShaderFilepath);
    Log(debug, fmt::terminal_color::bright_green, "created fragment shader module");
    vk::PipelineShaderStageCreateInfo fragmentShaderInfo;
    fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragmentShaderInfo.module = m_shader.fragmentShaderModule;
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
    pipelineLayout = CreatePipelineLayout();
    pipelineInfo.layout = pipelineLayout;

    renderPass = CreateRenderPass();
    pipelineInfo.renderPass = renderPass;

    graphicsPipeline = (device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
    Log(debug, fmt::terminal_color::bright_green, "created graphics pipeline");
}

vk::PipelineLayout GraphicsPipeline::CreatePipelineLayout()
{
    vk::PipelineLayoutCreateInfo layoutInfo;
    layoutInfo.setLayoutCount = 0;

    layoutInfo.pushConstantRangeCount = 1;
    vk::PushConstantRange pushConstantInfo;
    pushConstantInfo.offset = 0;
    pushConstantInfo.size = sizeof(ObjectData);
    pushConstantInfo.stageFlags = vk::ShaderStageFlagBits::eVertex;
    layoutInfo.pPushConstantRanges = &pushConstantInfo;

    return device.createPipelineLayout(layoutInfo);
}

vk::RenderPass GraphicsPipeline::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.format = swapchainDetails.imageFormat;
    colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachmentDesc.finalLayout = vk::ImageLayout::ePresentSrcKHR;

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

    return device.createRenderPass(renderPassInfo);
}

GraphicsPipeline::~GraphicsPipeline()
{
    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyRenderPass(renderPass);
}