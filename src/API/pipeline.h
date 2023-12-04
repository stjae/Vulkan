#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "../common.h"
#include "swapchain.h"
#include "shader.h"
#include "descriptor.h"

class GraphicsPipeline
{
public:
    GraphicsPipeline(const vk::Device& vkDevice, const SwapchainDetail& swapchainDetail)
        : vkDevice(vkDevice), swapchainDetail(swapchainDetail), shader(vkDevice), descriptor(vkDevice) {}
    void CreatePipeline();
    vk::PipelineLayout CreatePipelineLayout();
    vk::RenderPass CreateRenderPass();
    void CreateDescriptorPool();
    ~GraphicsPipeline();

    vk::Pipeline vkPipeline;
    vk::PipelineLayout vkPipelineLayout;
    vk::RenderPass vkRenderPass;
    const SwapchainDetail& swapchainDetail;
    const vk::Device& vkDevice;
    Shader shader;
    Descriptor descriptor;
    DescriptorSetLayoutData bindings;
};

#endif