#ifndef COLORIDPIPELINE_H
#define COLORIDPIPELINE_H

#include "../vulkan/pipeline.h"

class ColorIDPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    std::vector<vk::ClearValue> m_clearValues;

    void CreatePipeline() override;
    void UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateMeshUBO(const std::vector<vk::DescriptorBufferInfo>& bufferInfos);
} inline colorIDPipeline;

#endif
