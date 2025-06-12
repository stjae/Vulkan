#ifndef PHYSICSDEBUGPIPELINE_H
#define PHYSICSDEBUGPIPELINE_H

#include "bullet/btBulletCollisionCommon.h"
#include "../vulkan/pipeline.h"
#include "../scene/physicsDebugDrawer.h"
#include "../pipeline/meshRenderPipeline.h"

class PhysicsDebugPipeline : public vkn::Pipeline
{
    vk::VertexInputBindingDescription m_bindingDesc;
    std::array<vk::VertexInputAttributeDescription, 2> m_vertexInputAttribDesc;

    void SetUpDescriptors() override;
    void CreateRenderPass() override {};

public:
    PhysicsDebugPipeline();
    void CreatePipeline() override;
    void UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);
    void UpdateMeshUBO(const vk::DescriptorBufferInfo& bufferInfo);

} inline physicsDebugPipeline;

#endif