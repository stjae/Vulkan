#ifndef __CAMERA_H__
#define __CAMERA_H__

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
    friend class MyImGui;
    friend class Scene;

    bool isControllable = false;
    bool isInitial = true; // prevent sudden camera move

    glm::vec3 pos = { 0.0f, 0.0f, 4.0f };
    glm::vec4 dir = { 0.0f, 0.0f, -1.0f, 0.0f };
    glm::vec3 at = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 right = { 0.0f, 0.0f, 0.0f };

    glm::mat4 invProj;

    float speed = 4.0f;

    CameraMatrix matrix_;
    std::unique_ptr<Buffer> uniformBuffer_;
    void Update();
    void UpdateBuffer();
    vk::DescriptorBufferInfo& GetBufferInfo();

public:
    Camera();
};

#endif // __CAMERA_H__