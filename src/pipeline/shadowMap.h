#ifndef SHADOWMAPPIPELINE_H
#define SHADOWMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class ShadowMapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo m_shadowMapSpaceViewProjDescriptor;
    std::vector<vk::DescriptorBufferInfo> m_meshDescriptors;

    void CreatePipeline() override;
    void UpdateShadowMapSpaceViewProjDescriptor();
    void UpdateMeshDescriptors();
} inline shadowMapPipeline;

#endif
