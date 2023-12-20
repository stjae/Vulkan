#ifndef __IMGUI_H__
#define __IMGUI_H__

#include "common.h"
#include "camera.h"
#include "scene.h"
#include "../imgui/ImGuizmo.h"

class MyImGui
{
    vk::DescriptorPool imGuiDescriptorPool;
    std::weak_ptr<Scene> scene_;

public:
    void Setup(std::weak_ptr<Scene> scene, GraphicsPipeline& pipeline);
    void DrawImGuizmo(int currentItem);
    void DrawDockSpace();
    void SetCameraControl();
    void Draw();
    ~MyImGui();
};

#endif // __IMGUI_H__