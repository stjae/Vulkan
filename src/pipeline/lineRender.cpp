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
    vk::PipelineColorBlendAttachmentState attachmentState;
    m_shaderModule.m_vertexShaderModule = vkn::Shader::CreateModule("shader/spv/lineRender.vert.spv");
    m_shaderModule.m_fragmentShaderModule = vkn::Shader::CreateModule("shader/spv/lineRender.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, m_shaderModule.m_vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, m_shaderModule.m_fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();

    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, m_descriptorSetLayouts.size(), m_descriptorSetLayouts.data(), 0, nullptr);

    m_pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    m_pipelineCI.pStages = shaderStageInfos.data();
    m_colorBlendStateCI = { {}, vk::False, {}, 1, &attachmentState };
    m_pipelineCI.pColorBlendState = &m_colorBlendStateCI;
    m_pipelineCI.layout = vkn::Device::Get().device.createPipelineLayout(pipelineLayoutInfoCI);
    m_pipelineCI.renderPass = meshRenderPipeline.m_renderPass;
    m_multisampleStateCI.rasterizationSamples = vkn::Device::Get().maxSampleCount;

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
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(m_descriptorPool, poolSizes, maxSets);
    vkn::Descriptor::AllocateDescriptorSets(m_descriptorPool, m_descriptorSets, m_descriptorSetLayouts);
}

void LineRenderPipeline::UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo), nullptr);
}