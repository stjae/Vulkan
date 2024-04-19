#ifndef SHADOWCUBEMAPPIPELINE_H
#define SHADOWCUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class ShadowCubemapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo cameraDescriptor;
    vk::DescriptorBufferInfo lightDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;

    void CreatePipeline() override;
} inline shadowCubemapPipeline;

#endif
