// Rendering pipeline for line drawing
// ライン描画用レンダーパイプライン

#ifndef LINERENDERPIPELINE_H
#define LINERENDERPIPELINE_H

#include "../vulkan/pipeline.h"
#include "../scene/line.h"
#include "../pipeline/meshRenderPipeline.h"

class LineRenderPipeline : public vkn::Pipeline
{
    vk::VertexInputBindingDescription m_bindingDesc;
    std::array<vk::VertexInputAttributeDescription, 2> m_vertexInputAttribDesc;

    void SetUpDescriptors() override;
    void CreateRenderPass() override {};

public:
    LineRenderPipeline();
    void CreatePipeline() override;
    void UpdateCameraUBO(const vk::DescriptorBufferInfo& bufferInfo);

} inline lineRenderPipeline;

#endif