#include "pipeline.h"

void GraphicsPipeline::CreatePipeline()
{
    input.vertexShaderFilepath = "shaders/shader.vert.spv";
    input.fragmentShaderFilepath = "shaders/shader.frag.spv";
    input.swapchainExtent = Swapchain::swapchainDetails.extent;
    input.swapchainImageFormat = Swapchain::swapchainDetails.imageFormat;

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
    Shader::vertexShaderModule = m_shader.CreateModule(input.vertexShaderFilepath);
    vk::PipelineShaderStageCreateInfo vertexShaderInfo;
    vertexShaderInfo.setStage(vk::ShaderStageFlagBits::eVertex);
    vertexShaderInfo.setModule(Shader::vertexShaderModule);
    vertexShaderInfo.setPName("main");
    shaderStageInfos.push_back(vertexShaderInfo);

    // viewport and scissor
    vk::Viewport viewport;
    viewport.setX(0.0f);
    viewport.setY(0.0f);
    viewport.setWidth((float)input.swapchainExtent.width);
    viewport.setHeight((float)input.swapchainExtent.height);
    viewport.setMinDepth(0.0f);
    viewport.setMaxDepth(1.0f);

    vk::Rect2D scissor;
    scissor.setOffset(vk::Offset2D(0, 0));
    scissor.setExtent(input.swapchainExtent);

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
    Shader::fragmentShaderModule = m_shader.CreateModule(input.fragmentShaderFilepath);
    vk::PipelineShaderStageCreateInfo fragmentShaderInfo;
    fragmentShaderInfo.setStage(vk::ShaderStageFlagBits::eFragment);
    fragmentShaderInfo.setModule(Shader::fragmentShaderModule);
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
    std::array<float, 4Ui64> blendConstants{ 0.0f, 0.0f, 0.0f, 0.0f };
    colorBlendStateInfo.setBlendConstants(blendConstants);
    pipelineInfo.setPColorBlendState(&colorBlendStateInfo);

    // pipeline layout
    pipelineLayout = CreatePipelineLayout();
    pipelineInfo.setLayout(pipelineLayout);

    renderPass = CreateRenderPass();
    pipelineInfo.setRenderPass(renderPass);

    graphicsPipeline = (Device::device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
}

vk::PipelineLayout GraphicsPipeline::CreatePipelineLayout()
{
    vk::PipelineLayoutCreateInfo layoutInfo;
    layoutInfo.setSetLayoutCount(0);
    layoutInfo.setPushConstantRangeCount(0);

    return Device::device.createPipelineLayout(layoutInfo);
}

vk::RenderPass GraphicsPipeline::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.setFormat(input.swapchainImageFormat);
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

    return Device::device.createRenderPass(renderPassInfo);
}

GraphicsPipeline::~GraphicsPipeline()
{
    Device::device.destroyPipeline(graphicsPipeline);
    Device::device.destroyPipelineLayout(pipelineLayout);
    Device::device.destroyRenderPass(renderPass);
}

PipelineInDetails GraphicsPipeline::input;
PipelineOutDetails GraphicsPipeline::output;