#ifndef SKYBOXRENDERPIPELINE_H
#define SKYBOXRENDERPIPELINE_H

#include "../vulkan/pipeline.h"

class SkyboxRenderPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo m_cameraDescriptor;
    vk::DescriptorImageInfo m_irradianceCubemapDescriptor;

    void CreatePipeline() override;
    void UpdateCameraDescriptor();
    void UpdateIrradianceCubemapDescriptor();
} inline skyboxRenderPipeline;

#endif
