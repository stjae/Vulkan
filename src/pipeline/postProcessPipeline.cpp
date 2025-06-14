#include "postProcessPipeline.h"

void PostProcessPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    vk::PipelineColorBlendAttachmentState attachmentState;
    m_shaderModule.m_vertexShaderModule = vkn::Shader::CreateModule("shader/spv/postProcess.vert.spv");
    m_shaderModule.m_fragmentShaderModule = vkn::Shader::CreateModule("shader/spv/postProcess.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, m_shaderModule.m_vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, m_shaderModule.m_fragmentShaderModule, "main" };
    attachmentState = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    SetUpDescriptors();

    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eFragment, 0, sizeof(PostProcessPushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, m_descriptorSetLayouts.size(), m_descriptorSetLayouts.data(), 1, &pushConstantRange);

    m_pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    m_pipelineCI.pStages = shaderStageInfos.data();
    m_colorBlendStateCI = { {}, vk::False, {}, 1, &attachmentState };
    m_pipelineCI.pColorBlendState = &m_colorBlendStateCI;
    m_pipelineCI.layout = vkn::Device::Get().device.createPipelineLayout(pipelineLayoutInfoCI);
    m_pipelineCI.renderPass = meshRenderPipeline.m_renderPass;
    m_pipelineCI.subpass = 1;

    m_pipelineLayout = m_pipelineCI.layout;
    m_pipeline = (vkn::Device::Get().device.createGraphicsPipeline(nullptr, m_pipelineCI)).value;
}

void PostProcessPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // prev camera
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // camera
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // render image
        { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        // depth map
        { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
    };
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(m_descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
    vkn::Descriptor::AllocateDescriptorSets(m_descriptorPool, m_descriptorSets, m_descriptorSetLayouts);
}

void PostProcessPipeline::UpdatePrevCameraUBO(const vk::DescriptorBufferInfo& bufferInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo), nullptr);
}

void PostProcessPipeline::UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 1, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferInfo), nullptr);
}

void PostProcessPipeline::UpdateRenderImage(const vk::DescriptorImageInfo& imageInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 2, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo), nullptr);
}

void PostProcessPipeline::UpdateDepthMap(const vk::DescriptorImageInfo& imageInfo)
{
    vkn::Device::Get().device.updateDescriptorSets(vk::WriteDescriptorSet(m_descriptorSets[0], 3, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo), nullptr);
}
