#ifndef CAMERA_H
#define CAMERA_H

#include "../common.h"
#include "../window.h"
#include "../vulkan/buffer.h"
#include "../pipeline/meshRender.h"
#include "../../imgui/imgui.h"

struct CameraUBO
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 pos;
};

class Camera
{
    friend class Scene;
    friend class SceneSerializer;

    bool m_isControllable = false;
    bool m_isInitial = true; // prevent sudden camera move

    glm::vec3 m_pos = { 0.0f, 0.0f, 4.0f };
    glm::vec3 m_dir = { 0.0f, 0.0f, -1.0f };
    glm::vec3 m_at = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 m_right = { 0.0f, 0.0f, 0.0f };

    glm::mat4 m_invProj;

    float m_speed = 4.0f;

    CameraUBO m_cameraUBO;
    std::unique_ptr<vkn::Buffer> m_cameraBuffer;

    void SetCameraControl();
    void Update();

public:
    Camera();
    bool IsControllable() const { return m_isControllable; }
    const CameraUBO& GetMatrix() { return m_cameraUBO; }
    const vk::DescriptorBufferInfo& GetBufferInfo() { return m_cameraBuffer->Get().descriptorBufferInfo; }
};

#endif