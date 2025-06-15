// General object rendering pipeline
// オブジェクトレンダーパイプライン

#ifndef MESHRENDERPIPELINE_H
#define MESHRENDERPIPELINE_H

#include "../vulkan/pipeline.h"

class MeshRenderPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    std::vector<vk::ClearValue> m_clearValues;

    void CreatePipeline() override;
    void UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdatePointLightUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateMeshUBO(const std::vector<vk::DescriptorBufferInfo>& bufferInfos);
    void UpdateCascadeUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateSampler(const vk::DescriptorImageInfo& imageInfo);
    void UpdateAlbedoTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos);
    void UpdateNormalTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos);
    void UpdateMetallicTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos);
    void UpdateRoughnessTextures(const std::vector<vk::DescriptorImageInfo>& imageInfos);
    void UpdateShadowCubemap(const std::vector<vk::DescriptorImageInfo>& imageInfos);
    void UpdateIrraianceCubemap(const vk::DescriptorImageInfo& imageInfo);
    void UpdatePrefilteredCubemap(const vk::DescriptorImageInfo& imageInfo);
    void UpdateBrdfLut(const vk::DescriptorImageInfo& imageInfo);
    void UpdateShadowMap(const vk::DescriptorImageInfo& imageInfo);
} inline meshRenderPipeline;

#endif
