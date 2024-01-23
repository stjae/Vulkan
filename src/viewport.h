#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "common.h"
#include "API/image.h"
#include "API/commands.h"
#include "API/swapchain.h"

struct ViewportFrame
{
    vk::Framebuffer framebuffer;

    Image viewportImage;
    Image depthImage;

    Command command;
    Descriptor descriptor;
};

class Viewport
{
    Pipeline pipeline_;
    vk::RenderPass renderPass_;
    std::vector<DescriptorSetLayoutData> descriptorSetLayoutData_;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    vk::DescriptorPoolCreateFlags descriptorPoolCreateFlags_;

    void CreateDescriptorSetLayout();
    void CreateRenderPass();
    void CreateFrameBuffer();

public:
    std::vector<ViewportFrame> frames;
    vk::Extent2D extent;
    float panelRatio{};
    bool outDated{};

    Viewport();
    void RecordDrawCommand(size_t frameIndex, const std::vector<Mesh>& meshes, uint32_t dynamicOffsetSize);
    void RecreateViewportImages();
    ~Viewport();
};

#endif
