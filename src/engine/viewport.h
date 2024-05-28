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
#include "../scene/mesh.h"

class Viewport
{
    friend class UI;
    friend class Engine;

    vk::Framebuffer m_framebuffer;
    vkn::Image m_image;
    vkn::Image m_depth;
    vkn::Image m_colorID;

    vk::CommandPool m_commandPool;
    std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;
    std::array<vk::CommandBuffer, MAX_FRAME> m_pickColorCommandBuffers;
    vk::PipelineStageFlags m_waitStage = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    std::vector<vk::SubmitInfo> m_submitInfos;

    vkn::Image m_pickedColor;

public:
    vk::Extent2D m_extent;
    ImVec2 m_panelPos;
    ImVec2 m_panelSize;
    float m_panelRatio;
    bool m_outDated;
    bool m_isMouseHovered;

    Viewport();
    void CreateImage();
    void DestroyImage();
    void PickColor(double mouseX, double mouseY, Scene& scene);
    void Draw(const Scene& scene);
    const std::vector<vk::SubmitInfo>& GetSubmitInfos() { return m_submitInfos; }
    void ClearSubmitInfos() { m_submitInfos.clear(); }
    ~Viewport();
    void UpdateImage();
};

#endif
