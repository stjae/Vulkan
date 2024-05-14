#ifndef SHADOWCUBEMAPPIPELINE_H
#define SHADOWCUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class ShadowCubemapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo m_projDescriptor;
    vk::DescriptorBufferInfo m_pointLightDescriptor;
    std::vector<vk::DescriptorBufferInfo> m_meshDescriptors;

    void CreatePipeline() override;

    void UpdateProjDescriptor();
    void UpdatePointLightDescriptor();
    void UpdateMeshDescriptors();
} inline shadowCubemapPipeline;

#endif
