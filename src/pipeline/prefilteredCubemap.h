#ifndef PREFILTEREDCUBEMAPPIPELINE_H
#define PREFILTEREDCUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class PrefilteredCubemapPipeline : public vkn::Pipeline
{
    void CreateRenderPass() override;
    void SetUpDescriptors() override;

public:
    void CreatePipeline() override;
    void UpdateEnvCubemap(const vk::DescriptorImageInfo& imageInfo);
} inline prefilteredCubemapPipeline;

#endif
