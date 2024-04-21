#ifndef MESHRENDERPIPELINE_H
#define MESHRENDERPIPELINE_H

#include "../vulkan/pipeline.h"

struct MeshRenderPushConstants
{
    int meshIndex;
    int materialID;
    int lightCount;
    float iblExposure;
} inline meshRenderPushConsts;

class MeshRenderPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo cameraDescriptor;
    vk::DescriptorBufferInfo pointLightDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;

    vk::DescriptorBufferInfo dirLightDescriptor;
    vk::DescriptorImageInfo shadowMapImageDescriptor;
    vk::DescriptorBufferInfo shadowMapSpaceViewProjDescriptor;

    std::vector<vk::DescriptorImageInfo> albeodoTextureDescriptors;
    std::vector<vk::DescriptorImageInfo> normalTextureDescriptors;
    std::vector<vk::DescriptorImageInfo> metallicTextureDescriptors;
    std::vector<vk::DescriptorImageInfo> roughnessTextureDescriptors;
    std::vector<vk::DescriptorImageInfo> shadowCubemapDescriptors;

    vk::DescriptorImageInfo irradianceCubemapDescriptor;
    vk::DescriptorImageInfo prefilteredCubemapDescriptor;
    vk::DescriptorImageInfo brdfLutDescriptor;

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
