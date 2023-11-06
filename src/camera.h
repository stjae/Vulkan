#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "common.h"
#include "API/swapchain.h"

struct Camera {

    bool isLocked = false;

    glm::vec3 pos = { 0.0f, 0.0f, 4.0f };
    glm::vec4 dir = { 0.0f, 0.0f, -1.0f, 0.0f };
    glm::vec3 at = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 right = { 0.0f, 0.0f, 0.0f };

    glm::mat4 invProj;

    float speed = 4.0f;

    void Update(GLFWwindow* window)
    {

        static float delta, currentTime, lastTime = 0.0f;
        currentTime = glfwGetTime();

        delta = currentTime - lastTime;
        lastTime = currentTime;

        static double prevMouseX;
        static double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        mouseX /= width;
        mouseY /= height;
        mouseX -= 0.5f;
        mouseY -= 0.5f;

        glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), static_cast<float>(prevMouseX - mouseX), glm::vec3(0.0f, 1.0f, 0.0f));
        dir = rotateX * dir;
        glm::normalize(dir);
        at = pos + glm::vec3(dir.x, dir.y, dir.z);
        right = glm::cross(glm::vec3(dir.x, dir.y, dir.z), up);

        if (glfwGetKey(window, GLFW_KEY_W)) {
            pos += glm::vec3(dir.x, dir.y, dir.z) * delta * speed;
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            pos -= glm::vec3(dir.x, dir.y, dir.z) * delta * speed;
        }
        if (glfwGetKey(window, GLFW_KEY_A)) {
            pos -= right * delta * speed;
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            pos += right * delta * speed;
        }

        prevMouseX = mouseX;
    }
};

#endif // __CAMERA_H__