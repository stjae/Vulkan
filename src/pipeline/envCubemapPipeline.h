// Rendering pipeline for Environment Cubemap
// Environment Cubemapレンダーパイプライン

#ifndef ENVCUBEMAPPIPELINE_H
#define ENVCUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class EnvCubemapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
    void UpdateHDRimage(const vk::DescriptorImageInfo& imageInfo);
} inline envCubemapPipeline;

#endif
