#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "common.h"
#include "API/window.h"

struct Matrix {

    glm::mat4 view;
    glm::mat4 proj;
};

struct Camera {
    Camera(GLFWwindow* window);
    void Update(Window& window);

    bool isControllable = false;
    bool isInitial = true;

    glm::vec3 pos = { 0.0f, 0.0f, 4.0f };
    glm::vec4 dir = { 0.0f, 0.0f, -1.0f, 0.0f };
    glm::vec3 at = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 right = { 0.0f, 0.0f, 0.0f };

    glm::mat4 invProj;

    float speed = 4.0f;

    Matrix matrix;
};

#endif // __CAMERA_H__