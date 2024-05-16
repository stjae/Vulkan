#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "../common.h"
#include "../scene/scene.h"
#include "../vulkan/image.h"
#include "../vulkan/command.h"
#include "../vulkan/swapchain.h"
#include "../pipeline/meshRender.h"
#include "../pipeline/shadowMap.h"
#include "../pipeline/shadowCubemap.h"
#include "../pipeline/envCubemap.h"
#include "../pipeline/irradianceCubemap.h"
#include "../pipeline/brdfLut.h"
#include "../pipeline/skyboxRender.h"
#include "../pipeline/lineRender.h"
#include "../scene/mesh/mesh.h"

class Viewport
{
    struct Image
    {
        vk::Framebuffer framebuffer;
        vkn::Image image;
        vkn::Image depth;
        vkn::Image colorID;
    };

    vk::CommandPool m_commandPool;
    std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;
    vk::CommandBuffer m_pickColorCommandBuffer;
    vk::PipelineStageFlags m_waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    std::vector<vk::SubmitInfo> m_submitInfos;

    vkn::Image m_colorPicked;

public:
    std::vector<Image> m_images;
    vk::Extent2D m_extent;
    ImVec2 m_panelPos;
    ImVec2 m_panelSize;
    float m_panelRatio;
    bool m_outDated;
    bool m_isMouseHovered;

    Viewport();
    void CreateImages();
    void DestroyImages();
    void PickColor(double mouseX, double mouseY, uint32_t imageIndex);
    void Draw(const Scene& scene, uint32_t imageIndex);
    const std::vector<vk::SubmitInfo>& GetSubmitInfos() { return m_submitInfos; }
    void ClearSubmitInfos() { m_submitInfos.clear(); }
    ~Viewport();
    void UpdateImages();
};

#endif
