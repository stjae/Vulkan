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

    bool isControllable_ = false;
    bool isInitial_ = true; // prevent sudden camera move

    glm::vec3 pos_ = { 0.0f, 0.0f, 4.0f };
    glm::vec3 dir_ = { 0.0f, 0.0f, -1.0f };
    glm::vec3 at_ = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up_ = { 0.0f, 1.0f, 0.0f };
    glm::vec3 right_ = { 0.0f, 0.0f, 0.0f };

    glm::mat4 invProj_{};

    float speed_ = 4.0f;

    CameraUBO cameraUBO_{};
    std::unique_ptr<vkn::Buffer> cameraBuffer_;

    void SetCameraControl();
    void Update();

public:
    Camera();
    bool IsControllable() const { return isControllable_; }
    const CameraUBO& GetMatrix() { return cameraUBO_; }
    const vk::DescriptorBufferInfo& GetBufferInfo() { return cameraBuffer_->GetBundle().descriptorBufferInfo; }
};

#endif