#ifndef LINERENDERPIPELINE_H
#define LINERENDERPIPELINE_H

#include <bullet/btBulletCollisionCommon.h>
#include "../vulkan/pipeline.h"
#include "../scene/physicsDebugDrawer.h"

class LineRenderPipeline : public vkn::Pipeline
{
    vk::VertexInputBindingDescription m_bindingDesc;
    std::array<vk::VertexInputAttributeDescription, 2> m_vertexInputAttribDesc;

    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo m_cameraDescriptor;
    std::vector<vk::DescriptorBufferInfo> m_meshDescriptors;

    LineRenderPipeline();
    void CreatePipeline() override;
    void UpdateCameraDescriptor();
    void UpdateMeshDescriptors();

} inline lineRenderPipeline;

#endif