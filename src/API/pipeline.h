#ifndef PIPELINE_H
#define PIPELINE_H

#include "../common.h"
#include "shader.h"
#include "descriptor.h"
#include "../meshData.h"

class GraphicsPipeline
{
    Shader shader_;
    PipelineHandle handle_;

public:
    void CreatePipeline(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
    vk::PipelineLayout CreatePipelineLayout(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
    vk::RenderPass CreateRenderPass();
    ~GraphicsPipeline();

    const PipelineHandle& GetHandle() { return handle_; }
};

#endif