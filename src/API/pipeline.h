#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "../common.h"
#include "swapchain.h"
#include "shader.h"
#include "descriptor.h"

class GraphicsPipeline
{
    Shader shader_;
    DescriptorManager descriptorManager_;
    std::vector<DescriptorSetLayoutData> descriptorSetLayouts_;

    PipelineHandle handle_;

public:
    void CreatePipeline();
    vk::PipelineLayout CreatePipelineLayout();
    vk::RenderPass CreateRenderPass();
    void CreateDescriptorPool();
    void AllocateDescriptorSet(std::vector<vk::DescriptorSet>& descriptorSets);
    ~GraphicsPipeline();

    const PipelineHandle& GetHandle() { return handle_; }
};

#endif