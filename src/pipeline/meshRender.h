#ifndef MESHRENDERPIPELINE_H
#define MESHRENDERPIPELINE_H

#include "../vulkan/pipeline.h"

class MeshRenderPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo m_cameraDescriptor;
    vk::DescriptorBufferInfo m_pointLightDescriptor;
    std::vector<vk::DescriptorBufferInfo> m_meshDescriptors;

    vk::DescriptorBufferInfo m_dirLightDescriptor;
    vk::DescriptorImageInfo m_shadowMapImageDescriptor;
    vk::DescriptorBufferInfo m_shadowMapSpaceViewProjDescriptor;

    std::vector<vk::DescriptorImageInfo> m_albeodoTextureDescriptors;
    std::vector<vk::DescriptorImageInfo> m_normalTextureDescriptors;
    std::vector<vk::DescriptorImageInfo> m_metallicTextureDescriptors;
    std::vector<vk::DescriptorImageInfo> m_roughnessTextureDescriptors;
    std::vector<vk::DescriptorImageInfo> m_shadowCubemapDescriptors;

    vk::DescriptorImageInfo m_irradianceCubemapDescriptor;
    vk::DescriptorImageInfo m_prefilteredCubemapDescriptor;
    vk::DescriptorImageInfo m_brdfLutDescriptor;

    void CreatePipeline() override;

    void UpdateCameraDescriptor();
    void UpdatePointLightDescriptor();
    void UpdateMeshDescriptors();
    void UpdateDirLightDescriptor();
    void UpdateShadowMapDescriptor();
    void UpdateShadowMapSpaceViewProjDescriptor();
    void UpdateAlbedoTextureWriteDescriptors();
    void UpdateNormalTextureWriteDescriptors();
    void UpdateMetallicTextureWriteDescriptors();
    void UpdateRoughnessTextureWriteDescriptors();
    void UpdateShadowCubemapDescriptors();
    void UpdateIrraianceCubemapDescriptor();
    void UpdateBrdfLutDescriptor();
    void UpdatePrefilteredCubemapDescriptor();
} inline meshRenderPipeline;

#endif
