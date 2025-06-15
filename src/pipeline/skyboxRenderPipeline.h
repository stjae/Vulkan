// Rendering pipeline for background skybox drawing
// 背景描画用スカイボックスパイプライン

#ifndef SKYBOXRENDERPIPELINE_H
#define SKYBOXRENDERPIPELINE_H

#include "../vulkan/pipeline.h"
#include "../pipeline/meshRenderPipeline.h"

class SkyboxRenderPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override {};

public:
    void CreatePipeline() override;
    void UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateIrradianceCubemap(const vk::DescriptorImageInfo& imageInfo);
} inline skyboxRenderPipeline;

#endif
