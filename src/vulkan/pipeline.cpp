#include "pipeline.h"
#include "device.h"
#include "descriptor.h"
#include "shader.h"

namespace vkn {

Pipeline::Pipeline()
{
    m_bindingDesc = MeshBase::GetBindingDesc();
    m_vertexInputAttribDesc = MeshBase::GetAttributeDescs();
    m_vertexInputStateCI = { {}, 1, &m_bindingDesc, (uint32_t)m_vertexInputAttribDesc.size(), m_vertexInputAttribDesc.data() };
    m_inputAssemblyStateCI = { {}, vk::PrimitiveTopology::eTriangleList };
    m_dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    m_dynamicStateCI = { {}, (uint32_t)m_dynamicStates.size(), m_dynamicStates.data() };
    m_viewportStateCI = { {}, 1, {}, 1, {} };
    m_rasterizeStateCI = { {}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False, {}, {}, {}, 1.0f };
    m_multisampleStateCI = { {}, vk::SampleCountFlagBits::e1, vk::False };
    m_depthStencilStateCI = { {}, vk::True, vk::True, vk::CompareOp::eLess };

    m_pipelineCI.pVertexInputState = &m_vertexInputStateCI;
    m_pipelineCI.pInputAssemblyState = &m_inputAssemblyStateCI;
    m_pipelineCI.pDynamicState = &m_dynamicStateCI;
    m_pipelineCI.pViewportState = &m_viewportStateCI;
    m_pipelineCI.pRasterizationState = &m_rasterizeStateCI;
    m_pipelineCI.pMultisampleState = &m_multisampleStateCI;
    m_pipelineCI.pDepthStencilState = &m_depthStencilStateCI;
}

void Pipeline::Destroy()
{
    vkn::Device::Get().device.destroyPipeline(m_pipeline);
    vkn::Device::Get().device.destroyPipelineLayout(m_pipelineLayout);
    vkn::Device::Get().device.destroyRenderPass(m_renderPass);
    for (auto& descriptorSetLayout : m_descriptorSetLayouts)
        vkn::Device::Get().device.destroyDescriptorSetLayout(descriptorSetLayout);
    vkn::Device::Get().device.destroyDescriptorPool(m_descriptorPool);
    if (m_shaderModule.m_vertexShaderModule)
        vkn::Device::Get().device.destroyShaderModule(m_shaderModule.m_vertexShaderModule);
    if (m_shaderModule.m_fragmentShaderModule)
        vkn::Device::Get().device.destroyShaderModule(m_shaderModule.m_fragmentShaderModule);
}
} // namespace vkn
