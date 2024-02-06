#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "common.h"
#include "API/image.h"
#include "API/command.h"
#include "API/swapchain.h"

struct ViewportFrame
{
    vk::Framebuffer framebuffer;

    Image viewportImage;
    Image depthImage;
    Image colorID;

    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;

    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;
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
    ImVec2 panelPos{};
    ImVec2 panelSize{};
    float panelRatio{};
    bool outDated{};
    Image colorPicked_;

    Viewport();
    void CreateViewportImages();
    void DestroyViewportImages();
    int32_t PickColor(size_t frameIndex);
    void Draw(size_t frameIndex, const std::vector<Mesh>& meshes, uint32_t dynamicOffsetSize);
    ~Viewport();
};

#endif
