#ifndef IMGUI_H
#define IMGUI_H

#include "common.h"
#include "scene/camera.h"
#include "scene/scene.h"
#include "scene/light.h"
#include "viewport.h"
#include "../imgui/ImGuizmo.h"

class MyImGui
{
    vk::DescriptorPool descriptorPool_;
    std::vector<DescriptorBinding> descriptorBindings_;
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts_;
    std::vector<vk::DescriptorSet> descriptorSets_;

    void DrawDockSpace(Scene& scene);
    void DrawViewport(Scene& scene, Viewport& viewport, size_t frameIndex);
    void SetViewportUpToDate(Viewport& viewport, const ImVec2& viewportPanelSize);
    void DrawImGuizmo(Scene& scene, const ImVec2& viewportPanelPos);
    void DrawObjectWindow(Scene& scene);
    void DrawResourceWindow(Scene& scene);
    void ShowInformationOverlay(const Scene& scene);

public:
    void Setup(const vk::RenderPass& renderPass, Viewport& viewport);
    void Draw(Scene& scene, Viewport& viewport, size_t frameIndex);
    void RecreateViewportDescriptorSets(const Viewport& viewport);

    ~MyImGui();
};

#endif