#ifndef IMGUI_H
#define IMGUI_H

#include "../common.h"
#include "../scene/camera.h"
#include "../scene/scene.h"
#include "../scene/light.h"
#include "viewport.h"
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
    inline static std::vector<DescriptorBinding> descriptorBindings_;
    inline static std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    inline static std::vector<vk::DescriptorSet> descriptorSets_;

    std::unique_ptr<Image> plusIcon_;
    std::unique_ptr<Image> lightIcon_;
    std::unique_ptr<Image> cubeIcon_;
    vk::DescriptorSet plusIconDescriptorSet_;
    vk::DescriptorSet lightIconDescriptorSet_;
    vk::DescriptorSet cubeIconDescriptorSet_;

    void DrawDockSpace(Scene& scene);
    void DrawViewport(Scene& scene, Viewport& viewport, size_t frameIndex);
    void SetViewportUpToDate(Viewport& viewport, const ImVec2& viewportPanelSize);
    void DrawMeshGuizmo(Scene& scene, const ImVec2& viewportPanelPos);
    void DrawLightGuizmo(Scene& scene, const ImVec2& viewportPanelPos);
    void DrawListWindow(Scene& scene);
    void DrawResourceWindow(Scene& scene);
    void ShowInformationOverlay(const Scene& scene);

public:
    inline static ImDrawData* imDrawData;

    void Setup(const vk::RenderPass& renderPass, Viewport& viewport, Scene& scene);
    std::unique_ptr<Image> GenerateIcon(const std::string& filePath);
    void Draw(Scene& scene, Viewport& viewport, size_t frameIndex);
    void RecreateViewportDescriptorSets(const Viewport& viewport);
    ~UI();
};

#endif