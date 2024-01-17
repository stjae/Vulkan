#ifndef PIPELINE_H
#define PIPELINE_H

#include "../common.h"
#include "shader.h"
#include "descriptor.h"
#include "../meshData.h"

class Pipeline
{
    Shader shader_;
    PipelineHandle handle_;

public:
    void CreatePipeline(const vk::RenderPass& renderPass, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
    vk::PipelineLayout CreatePipelineLayout(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
    ~Pipeline();

    const PipelineHandle& GetHandle() { return handle_; }
};

#endif