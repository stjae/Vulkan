#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"
#include "API/window.h"
#include "API/buffer.h"

struct CameraMatrix
{
    glm::mat4 view;
    glm::mat4 proj;
};

class Camera
{
    //    friend class MyImGui;
    friend class Scene;

    bool isControllable_ = false;
    bool isInitial_ = true; // prevent sudden camera move

    glm::vec3 pos_ = { 0.0f, 0.0f, 4.0f };
    glm::vec4 dir_ = { 0.0f, 0.0f, -1.0f, 0.0f };
    glm::vec3 at_ = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up_ = { 0.0f, 1.0f, 0.0f };
    glm::vec3 right_ = { 0.0f, 0.0f, 0.0f };

    glm::mat4 invProj_;

    float speed_ = 4.0f;

    CameraMatrix matrix_;
    std::unique_ptr<Buffer> uniformBuffer_;

    void Update();
    void UpdateBuffer();
    const vk::DescriptorBufferInfo& GetBufferInfo();

public:
    Camera();
    bool IsControllable() { return isControllable_; }
    void SetCameraControl() { isControllable_ = !isControllable_; }
    void InitCamera() { isInitial_ = true; }
    const CameraMatrix& GetMatrix() { return matrix_; }
};

#endif