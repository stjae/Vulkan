#ifndef LINERENDERPIPELINE_H
#define LINERENDERPIPELINE_H

#include <bullet/btBulletCollisionCommon.h>
#include "../vulkan/pipeline.h"

struct LinePoint
{
    glm::vec3 pos;
    glm::vec3 color;
    LinePoint(const btVector3& pos, const btVector3& color) : pos(pos.x(), pos.y(), pos.z()), color(color.x(), color.y(), color.z()) {}
};

class LineRenderPipeline : public vkn::Pipeline
{
    vk::VertexInputBindingDescription bindingDesc_;
    std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttribDesc_;

    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    vk::DescriptorBufferInfo cameraDescriptor;

    LineRenderPipeline();
    void CreatePipeline() override;
    void UpdateCameraDescriptor();

} inline lineRenderPipeline;

#endif