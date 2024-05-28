#include "lineRender.h"

LineRenderPipeline::LineRenderPipeline()
{
    m_bindingDesc.setBinding(0);
    m_bindingDesc.setStride(sizeof(LinePoint));
    m_bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    uint32_t offset = 0;
    // Pos
    m_vertexInputAttribDesc[0].binding = 0;
    m_vertexInputAttribDesc[0].location = 0;
    m_vertexInputAttribDesc[0].format = vk::Format::eR32G32B32Sfloat;
    m_vertexInputAttribDesc[0].offset = offset;
    offset += sizeof(LinePoint::pos);
    // Color
    m_vertexInputAttribDesc[1].binding = 0;
    m_vertexInputAttribDesc[1].location = 1;
    m_vertexInputAttribDesc[1].format = vk::Format::eR32G32B32Sfloat;
    m_vertexInputAttribDesc[1].offset = offset;

    m_vertexInputStateCI = { {}, 1, &m_bindingDesc, (uint32_t)m_vertexInputAttribDesc.size(), m_vertexInputAttribDesc.data() };
    m_inputAssemblyStateCI = { {}, vk::PrimitiveTopology::eLineList };
    m_rasterizeStateCI = { {}, vk::False, vk::False, vk::PolygonMode::eLine, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, vk::False, {}, {}, {}, 1.0f };

    m_pipelineCI.pVertexInputState = &m_vertexInputStateCI;
    m_pipelineCI.pInputAssemblyState = &m_inputAssemblyStateCI;
    m_pipelineCI.pRasterizationState = &m_rasterizeStateCI;
}

void LineRenderPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    m_shaderModule.m_vertexShaderModule = vkn::Shader::CreateModule("shader/line.vert.spv");
    m_shaderModule.m_fragmentShaderModule = vkn::Shader::CreateModule("shader/line.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, m_shaderModule.m_vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, m_shaderModule.m_fragmentShaderModule, "main" };
    attachmentStates[0] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[1] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[1].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(LineRenderPushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, m_descriptorSetLayouts.size(), m_descriptorSetLayouts.data(), 1, &pushConstantRange);

    m_pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    m_pipelineCI.pStages = shaderStageInfos.data();
    m_colorBlendStateCI = { {}, vk::False, {}, attachmentStates.size(), attachmentStates.data() };
    m_pipelineCI.pColorBlendState = &m_colorBlendStateCI;
    m_pipelineCI.layout = vkn::Device::Get().device.createPipelineLayout(pipelineLayoutInfoCI);
    m_pipelineCI.renderPass = m_renderPass;

    m_pipelineLayout = m_pipelineCI.layout;
    m_pipeline = (vkn::Device::Get().device.createGraphicsPipeline(nullptr, m_pipelineCI)).value;
}

void LineRenderPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // mesh
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(m_descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(m_descriptorPool, m_descriptorSets, m_descriptorSetLayouts);
}

void LineRenderPipeline::CreateRenderPass()
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

    m_renderPass = vkn::Device::Get().device.createRenderPass(renderPassInfo);
}

void LineRenderPipeline::UpdateCameraDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[0];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &m_cameraDescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}

void LineRenderPipeline::UpdateUBODescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = m_descriptorSets[0];
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &m_UBODescriptor;
    vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}
