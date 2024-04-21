#ifndef SHADOWMAPPIPELINE_H
#define SHADOWMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class ShadowMapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo shadowMapSpaceViewProjDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;

    void CreatePipeline() override;
    void UpdateShadowMapSpaceViewProjDescriptor();
    void UpdateMeshDescriptors();
} inline shadowMapPipeline;

#endif
