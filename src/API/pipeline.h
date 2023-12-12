#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "../common.h"
#include "swapchain.h"
#include "shader.h"
#include "descriptor.h"

class GraphicsPipeline
{
    Shader shader;
    DescriptorManager descriptorManager;
    std::vector<DescriptorSetLayoutData> descriptorSetLayouts;

public:
    void CreatePipeline();
    vk::PipelineLayout CreatePipelineLayout();
    vk::RenderPass CreateRenderPass();
    void CreateDescriptorPool();
    void AllocateDescriptorSet(std::vector<vk::DescriptorSet>& descriptorSets);
    ~GraphicsPipeline();
    static vk::Pipeline& Pipeline()
    {
        static vk::Pipeline vkPipeline;
        return vkPipeline;
    }
    static vk::PipelineLayout& PipelineLayout()
    {
        static vk::PipelineLayout vkPipelineLayout;
        return vkPipelineLayout;
    }
    static vk::RenderPass& RenderPass()
    {
        static vk::RenderPass vkRenderPass;
        return vkRenderPass;
    }
};

#endif