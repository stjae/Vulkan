#ifndef CAMERA_H
#define CAMERA_H

#include "../common.h"
#include "../window.h"
#include "../vulkan/buffer.h"
#include "../pipeline/meshRender.h"
#include "../vulkan/sync.h"
#include "../vulkan/command.h"
#include "../../imgui/imgui.h"
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

    uint64_t m_assignedMeshInstanceID = 0;

    void SetControl();

public:
    explicit Camera();
    bool IsControllable() const { return m_isControllable; }
    const CameraUBO& GetUBO() { return m_cameraUBO; }
    virtual void Control() = 0;
    void Update(const vk::CommandBuffer& commandBuffer);
    float GetCascadeDepth(int index);
    glm::mat4 GetCascadeProj(int index);
    glm::vec3& GetRotation() { return m_rotation; }
    uint64_t GetAssignedMeshInstanceID();
};

class MainCamera : public Camera
{
    float m_speed = 4.0f;

public:
    void Control() override;
};

class SubCamera : public Camera
{
    bool m_isFirstFrame = true;

public:
    SubCamera(uint64_t meshInstanceID);
    void Control() override;
};

#endif