#ifndef IRRADIANCECUBEMAPPIPELINE_H
#define IRRADIANCECUBEMAPPIPELINE_H

#include "../vulkan/pipeline.h"

class IrradianceCubemapPipeline : public vkn::Pipeline
{
    void SetUpDescriptors() override;
    void CreateRenderPass() override;

public:
    void CreatePipeline() override;
} inline irradianceCubemapPipeline;

#endif
