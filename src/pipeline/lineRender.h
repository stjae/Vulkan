#ifndef LINERENDERPIPELINE_H
#define LINERENDERPIPELINE_H

#include <btBulletCollisionCommon.h>
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
    vk::DescriptorBufferInfo m_UBODescriptor;

    LineRenderPipeline();
    void CreatePipeline() override;
    void UpdateCameraDescriptor();
    void UpdateUBODescriptor();

} inline lineRenderPipeline;

#endif