#ifndef CAMERA_H
#define CAMERA_H

#include "../common.h"
#include "../window.h"
#include "../vulkan/buffer.h"
#include "../pipeline/meshRender.h"
#include "../../imgui/imgui.h"
#include "../vulkan/sync.h"
#include "../vulkan/command.h"
#include "../vulkan/swapchain.h"
#include "../../imgui/ImGuizmo.h"
#include "../time.h"

const uint8_t SHADOW_MAP_CASCADE_COUNT = 4;

struct CameraUBO
{
    glm::mat4 view{ 1.0f };
    glm::mat4 proj{ 1.0f };
    glm::vec3 pos{ 0.0f };
};

class Camera
{
    friend class UI;
    friend class Scene;
    friend class SceneSerializer;

    CameraUBO m_cameraUBO;
    std::unique_ptr<vkn::Buffer> m_cameraBuffer;
    std::unique_ptr<vkn::Buffer> m_cameraStagingBuffer;

protected:
    bool m_isControllable = false;
    bool m_startControl = true; // prevent sudden camera move

    float m_zNear = 0.1f;
    float m_zFar = 1024.0f;
    std::array<float, SHADOW_MAP_CASCADE_COUNT> m_cascadeRanges = { 18.0f, 40.0f, 85.0f, 120.0f };

    glm::vec3 m_pos = { 0.0f, 0.0f, 4.0f };
    glm::vec3 m_dir = { 0.0f, 0.0f, -1.0f };
    glm::vec3 m_at = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 m_right = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation = { 0.0f, 0.0f, 0.0f };

    void SetControl();

public:
    explicit Camera();
    bool IsControllable() const { return m_isControllable; }
    const CameraUBO& GetUBO() { return m_cameraUBO; }
    const vk::DescriptorBufferInfo& GetBufferInfo() { return m_cameraBuffer->Get().descriptorBufferInfo; }
    virtual void Control() = 0;
    virtual void ControlByMatrix(const glm::mat4& matrix) = 0;
    float GetZnear() const { return m_zNear; }
    float GetZfar() const { return m_zFar; }
    void Update(const vk::CommandBuffer& commandBuffer);
    float GetCascadeDepth(int index);
    glm::mat4 GetCascadeProj(int index);
    glm::vec3& GetDirection() { return m_dir; }
    glm::vec3& GetRotation() { return m_rotation; }
};

class MainCamera : public Camera
{
    float m_speed = 4.0f;

    void ControlByMatrix(const glm::mat4& matrix) override {}

public:
    void Control() override;
};

class SubCamera : public Camera
{
    bool m_isFirstFrame = true;

    void Control() override {}

public:
    void ControlByMatrix(const glm::mat4& matrix) override;
};

#endif