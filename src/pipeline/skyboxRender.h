#ifndef SKYBOXRENDER_H
#define SKYBOXRENDER_H

#include "../vulkan/pipeline.h"

struct SkyboxRenderPushConstants
{
    float exposure;
} inline skyboxRenderPushConstants;

class SkyboxRenderPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
} inline skyboxRenderPipeline;

#endif
