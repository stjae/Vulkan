#ifndef POSTPROCESSPIPELINE_H
#define POSTPROCESSPIPELINE_H

#include "../vulkan/pipeline.h"
#include "../pipeline/meshRenderPipeline.h"

class PostProcessPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override{};

public:
    void CreatePipeline() override;
    void UpdatePrevCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateDepthMap(const vk::DescriptorImageInfo& imageInfo);
    void UpdateRenderImage(const vk::DescriptorImageInfo& imageInfo);
} inline postProcessPipeline;

#endif
