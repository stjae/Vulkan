// Shadow cubemap rendering pipeline for point lights
// ポイントライト用シャドウキューブマップパイプライン

#ifndef SHADOWCUBEMAPPIPELINE_H
#define SHADOWCUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class ShadowCubemapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
    void UpdateProjBuffer(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdatePointLightUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateMeshUBO(const std::vector<vk::DescriptorBufferInfo>& bufferInfos);
};

extern ShadowCubemapPipeline shadowCubemapPipeline;

#endif
