// Shadow cubemap rendering pipeline for directional light
// ディレクショナルライト用シャドウキューブマップパイプライン

#ifndef SHADOWMAPPIPELINE_H
#define SHADOWMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class ShadowMapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
    void UpdateViewProjBuffer(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateMeshUBOBuffer(const std::vector<vk::DescriptorBufferInfo>& bufferInfos);
    //} inline shadowMapPipeline;
};

extern ShadowMapPipeline shadowMapPipeline;

#endif
