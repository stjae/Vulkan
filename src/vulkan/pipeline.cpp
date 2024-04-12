#include "pipeline.h"
#include "device.h"
#include "descriptor.h"
#include "shader.h"

namespace vkn {

Pipeline::Pipeline()
{
    bindingDesc_ = MeshBase::GetBindingDesc();
    vertexInputAttribDesc_ = MeshBase::GetAttributeDescs();
    vertexInputStateCI_ = { {}, 1, &bindingDesc_, (uint32_t)vertexInputAttribDesc_.size(), vertexInputAttribDesc_.data() };
    inputAssemblyStateCI_ = { {}, vk::PrimitiveTopology::eTriangleList };
    dynamicStates_ = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    dynamicStateCI_ = { {}, (uint32_t)dynamicStates_.size(), dynamicStates_.data() };
    viewportStateCI_ = { {}, 1, {}, 1, {} };
    rasterizeStateCI_ = { {}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False, {}, {}, {}, 1.0f };
    multisampleStateCI_ = { {}, vk::SampleCountFlagBits::e1, vk::False };
    depthStencilStateCI_ = { {}, vk::True, vk::True, vk::CompareOp::eLess };

    pipelineCI_.pVertexInputState = &vertexInputStateCI_;
    pipelineCI_.pInputAssemblyState = &inputAssemblyStateCI_;
    pipelineCI_.pDynamicState = &dynamicStateCI_;
    pipelineCI_.pViewportState = &viewportStateCI_;
    pipelineCI_.pRasterizationState = &rasterizeStateCI_;
    pipelineCI_.pMultisampleState = &multisampleStateCI_;
    pipelineCI_.pDepthStencilState = &depthStencilStateCI_;
}

Pipeline::~Pipeline()
{
    vkn::Device::GetBundle().device.destroyPipeline(pipeline);
    vkn::Device::GetBundle().device.destroyPipelineLayout(pipelineLayout);
    vkn::Device::GetBundle().device.destroyRenderPass(renderPass);
    for (auto& descriptorSetLayout : descriptorSetLayouts)
        vkn::Device::GetBundle().device.destroyDescriptorSetLayout(descriptorSetLayout);
    vkn::Device::GetBundle().device.destroyDescriptorPool(descriptorPool);
    if (shader.vertexShaderModule)
        vkn::Device::GetBundle().device.destroyShaderModule(shader.vertexShaderModule);
    if (shader.fragmentShaderModule)
        vkn::Device::GetBundle().device.destroyShaderModule(shader.fragmentShaderModule);
}

} // namespace vkn
