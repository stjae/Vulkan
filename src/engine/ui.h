#ifndef IMGUI_H
#define IMGUI_H

#include "../common.h"
#include "../scene/camera.h"
#include "../scene/scene.h"
#include "../scene/light.h"
#include "viewport.h"
#include "sceneSerializer.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_vulkan.h"
#if defined(_WIN32)
#include "../../imgui/imgui_impl_win32.h"
#endif
#include "../../imgui/ImGuizmo.h"
#include "../../imgui/imgui_internal.h"

class UI
{
    inline static vk::CommandPool commandPool_;
    inline static vk::CommandBuffer commandBuffer_;

    inline static vk::DescriptorPool descriptorPool_;
    inline static std::vector<vkn::DescriptorBinding> descriptorBindings_;
    inline static std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    inline static vk::DescriptorSet descriptorSet_;

    vkn::Image plusIcon_;
    vkn::Image lightIcon_;
    vkn::Image cubeIcon_;
    vk::DescriptorSet plusIconDescriptorSet_;
    vk::DescriptorSet lightIconDescriptorSet_;
    vk::DescriptorSet cubeIconDescriptorSet_;
    vk::DescriptorSet hdriThumbnailDescriptorSet_;

    void DrawDockSpace(Scene& scene);
    void DrawViewport(Scene& scene, Viewport& viewport, size_t frameIndex);
    void SetViewportUpToDate(Viewport& viewport, const ImVec2& viewportPanelSize);
    void DrawMeshGuizmo(Scene& scene, const ImVec2& viewportPanelPos);
    void DrawLightGuizmo(Scene& scene, const ImVec2& viewportPanelPos);
    void DrawSceneAttribWindow(Scene& scene);
    void DrawResourceWindow(Scene& scene);
    void ShowInformationOverlay(const Scene& scene);

public:
    inline static bool dragDropped;
    inline static double dragDropMouseX;
    inline static double dragDropMouseY;
    inline static std::unique_ptr<Resource> dragDropResource;
    inline static ImDrawData* imDrawData;
    inline static float buttonSize = 100.0f;
    inline static float padding = buttonSize * 0.4f;
    inline static float buttonSizeWithoutPadding = buttonSize - padding * 2.0f;

    void Setup(const vk::RenderPass& renderPass, Viewport& viewport, Scene& scene);
    void Draw(Scene& scene, Viewport& viewport, size_t frameIndex);
    void RecreateViewportDescriptorSets(const Viewport& viewport);
    void AcceptDragDrop(Viewport& viewport, Scene& scene, size_t frameIndex);
    ~UI();
};

#endif