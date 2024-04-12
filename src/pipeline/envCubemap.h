#ifndef ENVCUBEMAPPIPELINE_H
#define ENVCUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class EnvCubemapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
} inline envCubemapPipeline;

#endif
