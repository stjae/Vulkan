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

    CameraUBO m_cameraUBO;
    std::unique_ptr<vkn::Buffer> m_cameraBuffer;
    std::unique_ptr<vkn::Buffer> m_cameraStagingBuffer;
    std::array<vk::CommandBuffer, MAX_FRAME> m_commandBuffers;

protected:
    bool m_isControllable = false;
    bool m_startControl = true; // prevent sudden camera move

    glm::vec3 m_pos = { 0.0f, 0.0f, 4.0f };
    glm::vec3 m_dir = { 0.0f, 0.0f, -1.0f };
    glm::vec3 m_at = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 m_right = { 0.0f, 0.0f, 0.0f };

    void SetControl();

public:
    explicit Camera(const vk::CommandPool& commandPool);
    bool IsControllable() const { return m_isControllable; }
    const CameraUBO& GetUBO() { return m_cameraUBO; }
    const vk::DescriptorBufferInfo& GetBufferInfo() { return m_cameraBuffer->Get().descriptorBufferInfo; }
    virtual void Control() = 0;
    virtual void ControlByMatrix(const glm::mat4& matrix) = 0;
    void Update();
};

class MainCamera : public Camera
{
    float m_speed = 4.0f;

    void ControlByMatrix(const glm::mat4& matrix) override {}

public:
    MainCamera(const vk::CommandPool& commandPool) : Camera(commandPool) {}
    void Control() override;
};

class SubCamera : public Camera
{
    bool m_isFirstFrame = true;

    void Control() override {}

public:
    SubCamera(const vk::CommandPool& commandPool) : Camera(commandPool) {}
    void ControlByMatrix(const glm::mat4& matrix) override;
};

#endif