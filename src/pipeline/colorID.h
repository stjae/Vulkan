#ifndef COLORIDPIPELINE_H
#define COLORIDPIPELINE_H

#include "../vulkan/pipeline.h"

class ColorIDPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
    void UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateMeshUBO(const std::vector<vk::DescriptorBufferInfo>& bufferInfos);
} inline colorIDPipeline;

#endif
