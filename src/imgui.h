#ifndef __IMGUI_H__
#define __IMGUI_H__

#include "common.h"
#include "engine.h"
#include "camera.h"
#include "scene.h"
#include <imguizmo/ImGuizmo.h>
#include <bitset>

class MyImGui
{
public:
    void Setup(const std::unique_ptr<GraphicsEngine>& engine);
    void DrawImGuizmo(Camera& camera, const std::unique_ptr<Scene>& scene);
    void DrawDockSpace();
    void ControlCamera(Camera& camera, GLFWwindow* window);
    void Draw(Camera& camera, const std::unique_ptr<Scene>& scene, GLFWwindow* window);
};

#endif // __IMGUI_H__