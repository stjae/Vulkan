#ifndef IMGUI_H
#define IMGUI_H

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_vulkan.h"
#if defined(_WIN32)
#include "../../imgui/imgui_impl_win32.h"
#endif
#include "../../imgui/ImGuizmo.h"
#include "../../imgui/imgui_internal.h"

#include "../common.h"
#include "../scene/camera.h"
#include "../scene/scene.h"
#include "viewport.h"
#include "sceneSerializer.h"
#include "../engine/script/script.h"
#include "../time.h"

class UI
{
    vk::CommandPool m_commandPool;
    vk::CommandBuffer m_commandBuffer;

    vk::DescriptorPool m_descriptorPool;
    std::vector<vkn::DescriptorBinding> m_descriptorBindings;
    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
    vk::DescriptorSet m_viewportImageDescriptorSet;

    vkn::Image m_plusIcon;
    vkn::Image m_lightIcon;
    vkn::Image m_cameraIcon;
    vkn::Image m_noCameraIcon;
    vkn::Image m_cubeIcon;
    vkn::Image m_playIcon;
    vkn::Image m_stopIcon;
    vk::DescriptorSet m_plusIconDescriptorSet;
    vk::DescriptorSet m_lightIconDescriptorSet;
    vk::DescriptorSet m_cameraIconDescriptorSet;
    vk::DescriptorSet m_noCameraIconDescriptorSet;
    vk::DescriptorSet m_cubeIconDescriptorSet;
    vk::DescriptorSet m_playIconDescriptorSet;
    vk::DescriptorSet m_stopIconDescriptorSet;
    // TODO:
    // vk::DescriptorSet m_hdriThumbnailDescriptorSet;
    // vk::DescriptorSet m_shadowMapDescriptorSet;

    void DrawDockSpace(Scene& scene, bool& init);
    void DrawViewport(Scene& scene, Viewport& viewport);
    void DrawMeshGuizmo(Scene& scene, const Viewport& viewport);
    void DrawLightGuizmo(Scene& scene, const Viewport& viewport);
    void DrawSceneAttribWindow(Scene& scene);
    void DrawResourceWindow(Scene& scene);
    void ShowInformationOverlay(const Scene& scene);
    void DrawLightIcon(const Scene& scene, const Viewport& viewport);
    void DrawCameraIcon(const Scene& scene, const Viewport& viewport);

    static float GetButtonPadding(float desiredButtonSize, float paddingRatio) { return desiredButtonSize * paddingRatio; }
    static float GetIconSize(float desiredButtonSize, float padding) { return desiredButtonSize - padding * 2.0f; }

public:
    inline static bool s_dragDropped;
    inline static double s_dragDropMouseX;
    inline static double s_dragDropMouseY;
    inline static std::unique_ptr<Resource> s_dragDropResource;

    void Init(const vk::RenderPass& renderPass, Viewport& viewport, Scene& scene);
    void Draw(Scene& scene, Viewport& viewport, bool& init);
    void RecreateViewportDescriptorSet(const Viewport& viewport);
    void AcceptDragDrop(Viewport& viewport, Scene& scene);
    void DrawInitPopup(bool& init, Scene& scene);
    ~UI();
};

#endif