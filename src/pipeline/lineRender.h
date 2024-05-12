#ifndef LINERENDERPIPELINE_H
#define LINERENDERPIPELINE_H

#include <bullet/btBulletCollisionCommon.h>
#include "../vulkan/pipeline.h"
#include "../scene/physicsDebugDrawer.h"

struct LineRenderPushConstants
{
    int meshIndex;
} inline lineRenderPushConsts;

class LineRenderPipeline : public vkn::Pipeline
{
    vk::VertexInputBindingDescription bindingDesc_;
    std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttribDesc_;

    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo cameraDescriptor;
    std::vector<vk::DescriptorBufferInfo> meshDescriptors;

    LineRenderPipeline();
    void CreatePipeline() override;
    void UpdateCameraDescriptor();
    void UpdateMeshDescriptors();

} inline lineRenderPipeline;

#endif