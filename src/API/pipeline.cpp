#include "pipeline.h"

void GraphicsPipeline::CreatePipeline()
{
    std::string vertexShaderFilepath = "shaders/shader.vert.spv";
    std::string fragmentShaderFilepath = "shaders/shader.frag.spv";

    vk::GraphicsPipelineCreateInfo pipelineInfo;

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

    // vertex input
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.setVertexBindingDescriptionCount(0);
    vertexInputInfo.setVertexAttributeDescriptionCount(0);
    pipelineInfo.setPVertexInputState(&vertexInputInfo);

    // input assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
    pipelineInfo.setPInputAssemblyState(&inputAssemblyInfo);

    // vertex shader
    m_shader.vertexShaderModule = m_shader.CreateModule(vertexShaderFilepath);
    Log(debug, fmt::terminal_color::bright_green, "created vertex shader module");
    vk::PipelineShaderStageCreateInfo vertexShaderInfo;
    vertexShaderInfo.setStage(vk::ShaderStageFlagBits::eVertex);
    vertexShaderInfo.setModule(m_shader.vertexShaderModule);
    vertexShaderInfo.setPName("main");
    shaderStageInfos.push_back(vertexShaderInfo);

    // viewport and scissor
    vk::Viewport viewport;
    viewport.setX(0.0f);
    viewport.setY(0.0f);
    viewport.setWidth((float)swapchainDetails.extent.width);
    viewport.setHeight((float)swapchainDetails.extent.height);
    viewport.setMinDepth(0.0f);
    viewport.setMaxDepth(1.0f);

    vk::Rect2D scissor;
    scissor.setOffset(vk::Offset2D(0, 0));
    scissor.setExtent(swapchainDetails.extent);

    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.setViewportCount(1);
    viewportState.setPViewports(&viewport);
    viewportState.setScissorCount(1);
    viewportState.setPScissors(&scissor);
    pipelineInfo.setPViewportState(&viewportState);

    // rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
    rasterizerInfo.setDepthClampEnable(VK_FALSE);
    rasterizerInfo.setRasterizerDiscardEnable(VK_FALSE);
    rasterizerInfo.setPolygonMode(vk::PolygonMode::eFill);
    rasterizerInfo.setLineWidth(1.0f);
    rasterizerInfo.setCullMode(vk::CullModeFlagBits::eBack);
    rasterizerInfo.setFrontFace(vk::FrontFace::eClockwise);
    rasterizerInfo.setDepthBiasEnable(VK_FALSE);
    pipelineInfo.setPRasterizationState(&rasterizerInfo);

    // fragment shader
    m_shader.fragmentShaderModule = m_shader.CreateModule(fragmentShaderFilepath);
    Log(debug, fmt::terminal_color::bright_green, "created fragment shader module");
    vk::PipelineShaderStageCreateInfo fragmentShaderInfo;
    fragmentShaderInfo.setStage(vk::ShaderStageFlagBits::eFragment);
    fragmentShaderInfo.setModule(m_shader.fragmentShaderModule);
    fragmentShaderInfo.setPName("main");
    shaderStageInfos.push_back(fragmentShaderInfo);
    pipelineInfo.setStageCount((uint32_t)shaderStageInfos.size());
    pipelineInfo.setPStages(shaderStageInfos.data());

    // multisampling
    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
    multisampleStateInfo.setSampleShadingEnable(VK_FALSE);
    multisampleStateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);
    pipelineInfo.setPMultisampleState(&multisampleStateInfo);

    // color blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
    colorBlendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR |
                                                vk::ColorComponentFlagBits::eG |
                                                vk::ColorComponentFlagBits::eB |
                                                vk::ColorComponentFlagBits::eA);
    colorBlendAttachmentState.setBlendEnable(VK_FALSE);
    vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;
    colorBlendStateInfo.setLogicOpEnable(VK_FALSE);
    colorBlendStateInfo.setAttachmentCount(1);
    colorBlendStateInfo.setPAttachments(&colorBlendAttachmentState);
    std::array<float, 4> blendConstants{ 0.0f, 0.0f, 0.0f, 0.0f };
    colorBlendStateInfo.setBlendConstants(blendConstants);
    pipelineInfo.setPColorBlendState(&colorBlendStateInfo);

    // pipeline layout
    pipelineLayout = CreatePipelineLayout();
    pipelineInfo.setLayout(pipelineLayout);

    renderPass = CreateRenderPass();
    pipelineInfo.setRenderPass(renderPass);

    graphicsPipeline = (device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
    Log(debug, fmt::terminal_color::bright_green, "created graphics pipeline");
}

vk::PipelineLayout GraphicsPipeline::CreatePipelineLayout()
{
    vk::PipelineLayoutCreateInfo layoutInfo;
    layoutInfo.setSetLayoutCount(0);

    layoutInfo.setPushConstantRangeCount(1);
    vk::PushConstantRange pushConstantInfo;
    pushConstantInfo.setOffset(0);
    pushConstantInfo.setSize(sizeof(ObjectData));
    pushConstantInfo.setStageFlags(vk::ShaderStageFlagBits::eVertex);
    layoutInfo.setPPushConstantRanges(&pushConstantInfo);

    return device.createPipelineLayout(layoutInfo);
}

vk::RenderPass GraphicsPipeline::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.setFormat(swapchainDetails.imageFormat);
    colorAttachmentDesc.setSamples(vk::SampleCountFlagBits::e1);
    colorAttachmentDesc.setLoadOp(vk::AttachmentLoadOp::eClear);
    colorAttachmentDesc.setStoreOp(vk::AttachmentStoreOp::eStore);
    colorAttachmentDesc.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    colorAttachmentDesc.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    colorAttachmentDesc.setInitialLayout(vk::ImageLayout::eUndefined);
    colorAttachmentDesc.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.setAttachment(0);
    colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpassDesc;
    subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpassDesc.setColorAttachmentCount(1);
    subpassDesc.setPColorAttachments(&colorAttachmentRef);

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.setAttachmentCount(1);
    renderPassInfo.setPAttachments(&colorAttachmentDesc);
    renderPassInfo.setSubpassCount(1);
    renderPassInfo.setPSubpasses(&subpassDesc);

    return device.createRenderPass(renderPassInfo);
}

GraphicsPipeline::~GraphicsPipeline()
{
    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyRenderPass(renderPass);
}