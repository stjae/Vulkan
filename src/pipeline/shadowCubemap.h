#ifndef SHADOWCUBEMAPPIPELINE_H
#define SHADOWCUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class ShadowCubemapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo projDescriptor;
    vk::DescriptorBufferInfo pointLightDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;

    void CreatePipeline() override;

    void UpdateProjDescriptor();
    void UpdatePointLightDescriptor();
    void UpdateMeshDescriptors();
} inline shadowCubemapPipeline;

#endif
