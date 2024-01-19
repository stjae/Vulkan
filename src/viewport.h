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
public:
    std::vector<ViewportFrame> frames_;

    Pipeline pipeline_;
    vk::RenderPass renderPass_;
    std::vector<DescriptorSetLayoutData> descriptorSetLayoutData_;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    vk::DescriptorPoolCreateFlags descriptorPoolCreateFlags_;

    vk::Extent2D extent_;
    float panelRatio_{};
    ImVec2 panelSize_;
    bool outDated_;

    void CreateDescriptorSetLayout();
    void CreateRenderPass();
    void CreateFrameBuffer();
    void RecreateViewportImages();

    Viewport();
    void RecordDrawCommand(size_t frameIndex, const std::vector<Mesh>& meshes, uint32_t dynamicOffsetSize);
    ~Viewport();
};

#endif
