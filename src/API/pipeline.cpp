#include "pipeline.h"

void GraphicsPipeline::CreatePipeline()
{
    std::string vertexShaderFilepath = "shaders/shader.vert.spv";
    std::string fragmentShaderFilepath = "shaders/shader.frag.spv";

    vk::GraphicsPipelineCreateInfo pipelineInfo;

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

    // vertex input
    vk::VertexInputBindingDescription bindingDesc = MeshData::GetBindingDesc();
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescs = MeshData::GetAttributeDescs();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
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
    viewportState.scissorCount = 1;
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

    // depth stencil
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
    depthStencilInfo.depthTestEnable = vk::True;
    depthStencilInfo.depthWriteEnable = vk::True;
    depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
    depthStencilInfo.depthBoundsTestEnable = vk::False;
    depthStencilInfo.stencilTestEnable = vk::False;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;

    // pipeline layout
    GraphicsPipeline::PipelineLayout() = CreatePipelineLayout();
    pipelineInfo.layout = GraphicsPipeline::PipelineLayout();

    GraphicsPipeline::RenderPass() = CreateRenderPass();
    pipelineInfo.renderPass = GraphicsPipeline::RenderPass();

    GraphicsPipeline::Pipeline() = (Device::GetDevice().createGraphicsPipeline(nullptr, pipelineInfo)).value;
    Log(debug, fmt::terminal_color::bright_green, "created graphics pipeline");
}

vk::PipelineLayout GraphicsPipeline::CreatePipelineLayout()
{
    DescriptorSetLayoutData layout0;
    layout0.descriptorSetCount = 1;

    // descriptor set layout #0
    layout0.indices.push_back(0);
    layout0.descriptorTypes.push_back(vk::DescriptorType::eUniformBuffer);
    layout0.descriptorCounts.push_back(1);
    layout0.bindingStages.push_back(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    descriptorManager.CreateSetLayout(layout0, nullptr);
    descriptorSetLayouts.push_back(layout0);

    DescriptorSetLayoutData layout1;
    layout1.descriptorSetCount = 1;

    // descriptor set layout #1
    layout1.indices.push_back(0);
    layout1.descriptorTypes.push_back(vk::DescriptorType::eUniformBufferDynamic);
    layout1.descriptorCounts.push_back(1);
    layout1.bindingStages.push_back(vk::ShaderStageFlagBits::eVertex);
    descriptorManager.CreateSetLayout(layout1, nullptr);
    descriptorSetLayouts.push_back(layout1);

    DescriptorSetLayoutData layout2;
    layout2.descriptorSetCount = 1;

    // descriptor set layout #2
    layout2.indices.push_back(0);
    layout2.descriptorTypes.push_back(vk::DescriptorType::eCombinedImageSampler);
    layout2.descriptorCounts.push_back(2);
    layout2.bindingStages.push_back(vk::ShaderStageFlagBits::eFragment);
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT setLayoutBindingFlags{};
    setLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    setLayoutBindingFlags.bindingCount = 1;
    std::vector<VkDescriptorBindingFlagsEXT> descriptorBindingFlags = {
        0,
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
    };
    setLayoutBindingFlags.pBindingFlags = descriptorBindingFlags.data();

    descriptorManager.CreateSetLayout(layout2, &setLayoutBindingFlags);
    descriptorSetLayouts.push_back(layout2);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    vk::PushConstantRange range(vk::ShaderStageFlagBits::eFragment, 0, sizeof(uint32_t));
    pipelineLayoutInfo.pPushConstantRanges = &range;

    pipelineLayoutInfo.setLayoutCount = descriptorManager.descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorManager.descriptorSetLayouts.data();

    return Device::GetDevice().createPipelineLayout(pipelineLayoutInfo);
}

vk::RenderPass GraphicsPipeline::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.format = Swapchain::GetDetail().imageFormat;
    colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentDescription depthAttachmentDesc;
    depthAttachmentDesc.format = Swapchain::GetDetail().depthImageFormat;
    depthAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    depthAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eClear;
    depthAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachmentDesc.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<vk::AttachmentDescription, 2> attachments = { colorAttachmentDesc, depthAttachmentDesc };
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    return Device::GetDevice().createRenderPass(renderPassInfo);
}

void GraphicsPipeline::CreateDescriptorPool()
{
    descriptorManager.CreatePool(static_cast<uint32_t>(Swapchain::GetDetail().frames.size()), descriptorSetLayouts);
}

void GraphicsPipeline::AllocateDescriptorSet(std::vector<vk::DescriptorSet>& descriptorSets)
{
    descriptorManager.AllocateSet(descriptorSets);
}

GraphicsPipeline::~GraphicsPipeline()
{
    Device::GetDevice().destroyPipeline(GraphicsPipeline::Pipeline());
    Device::GetDevice().destroyPipelineLayout(GraphicsPipeline::PipelineLayout());
    Device::GetDevice().destroyRenderPass(GraphicsPipeline::RenderPass());
    Device::GetDevice().destroyDescriptorPool(descriptorManager.GetDescriptorPool());
}