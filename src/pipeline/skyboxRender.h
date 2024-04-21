#ifndef SKYBOXRENDERPIPELINE_H
#define SKYBOXRENDERPIPELINE_H

#include "../vulkan/pipeline.h"

struct SkyboxRenderPushConstants
{
    float exposure;
} inline skyboxRenderPushConstants;

class SkyboxRenderPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo cameraDescriptor;
    vk::DescriptorImageInfo irradianceCubemapDescriptor;

    void CreatePipeline() override;
    void UpdateCameraDescriptor();
    void UpdateIrradianceCubemapDescriptor();
} inline skyboxRenderPipeline;

#endif
