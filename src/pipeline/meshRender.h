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
    vk::DescriptorBufferInfo lightDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;

    void CreatePipeline() override;

} inline meshRenderPipeline;

#endif
