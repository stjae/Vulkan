#ifndef SHADOWMAPPIPELINE_H
#define SHADOWMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class ShadowMapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo viewProjDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;

    void CreatePipeline() override;
} inline shadowMapPipeline;

#endif
