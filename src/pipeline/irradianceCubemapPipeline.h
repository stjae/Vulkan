#ifndef IRRADIANCECUBEMAPPIPELINE_H
#define IRRADIANCECUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class IrradianceCubemapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
    void UpdateEnvCubemap(const vk::DescriptorImageInfo& imageInfo);
} inline irradianceCubemapPipeline;

#endif
