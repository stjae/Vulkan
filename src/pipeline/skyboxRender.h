#ifndef SKYBOXRENDERPIPELINE_H
#define SKYBOXRENDERPIPELINE_H

#include "../vulkan/pipeline.h"
#include "../pipeline/meshRender.h"

class SkyboxRenderPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override{};

public:
    void CreatePipeline() override;
    void UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateIrradianceCubemap(const vk::DescriptorImageInfo& imageInfo);
} inline skyboxRenderPipeline;

#endif
