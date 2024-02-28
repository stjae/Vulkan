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
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;

public:
    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    void CreateGraphicsPipeline(const vk::RenderPass& renderPass, const char* vertexShaderFilepath, const char* fragmentShaderFilepath, vk::PrimitiveTopology topology);
    void CreateMeshRenderDescriptorSetLayout();
    ~Pipeline();

    const PipelineBundle& GetBundle() { return bundle_; }
};

#endif