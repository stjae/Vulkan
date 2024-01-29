#ifndef PIPELINE_H
#define PIPELINE_H

#include "../common.h"
#include "shader.h"
#include "descriptor.h"
#include "../scene/meshData.h"

struct PipelineBundle
{
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
};

class Pipeline
{
    Shader shader_;
    PipelineBundle bundle_;

public:
    void CreatePipeline(const vk::RenderPass& renderPass, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
    vk::PipelineLayout CreatePipelineLayout(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
    ~Pipeline();

    const PipelineBundle& GetBundle() { return bundle_; }
};

#endif