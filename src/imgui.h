#ifndef IMGUI_H
#define IMGUI_H

#include "common.h"
#include "camera.h"
#include "scene.h"
#include "viewport.h"
#include "../imgui/ImGuizmo.h"

class MyImGui
{
    Descriptor descriptor_;
    std::vector<DescriptorSetLayoutData> descriptorSetLayoutData_;
    std::vector<vk::DescriptorSet> viewportDescriptorSets_;

    int itemSelected_ = -1;

public:
    void Setup(const vk::RenderPass& renderPass, Viewport& viewport);
    void Draw(std::unique_ptr<Scene>& scene, Viewport& viewport, size_t frameIndex);
    void DrawDockSpace(std::unique_ptr<Scene>& scene, Viewport& viewport, size_t frameIndex);
    void DrawViewport(std::unique_ptr<Scene>& scene, Viewport& viewport, size_t frameIndex);
    void DrawImGuizmo(std::unique_ptr<Scene>& scene, ImVec2& viewportPanelPos);
    void DrawObjectListWindow(std::unique_ptr<Scene>& scene);
    void DrawObjectAttribWindow(std::unique_ptr<Scene>& scene);
    void DrawResourceWindow(std::unique_ptr<Scene>& scene);
    void ShowInformationOverlay(std::unique_ptr<Scene>& scene);
    void RecreateViewportDescriptorSets(Viewport& viewport);
    void SetCameraControl(std::unique_ptr<Scene>& scene);
    ~MyImGui();
};

#endif