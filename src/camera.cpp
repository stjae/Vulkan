#include "camera.h"

Camera::Camera(GLFWwindow* window)
{
    glfwSetWindowUserPointer(window, this);
}

void Camera::Update(Window& window)
{
    static double delta, currentTime, lastTime = 0.0;
    currentTime = glfwGetTime();

    delta = currentTime - lastTime;
    lastTime = currentTime;

    static double prevMouseX, prevMouseY;
    static double mouseX, mouseY;
    glfwGetCursorPos(window.window, &mouseX, &mouseY);
    mouseX /= window.width;
    mouseY /= window.height;
    mouseX -= 0.5f;
    mouseY -= 0.5f;

    if (isInitial) {
        prevMouseX = mouseX;
        prevMouseY = mouseY;

        isInitial = false;
    }

    glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), static_cast<float>(prevMouseX - mouseX), up);
    dir = rotateX * dir;
    glm::normalize(dir);
    right = glm::cross(glm::vec3(dir.x, dir.y, dir.z), up);
    glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), static_cast<float>(prevMouseY - mouseY), right);
    dir = rotateY * dir;

    if (glfwGetKey(window.window, GLFW_KEY_W)) {
        pos += glm::vec3(dir.x, dir.y, dir.z) * static_cast<float>(delta) * speed;
    }
    if (glfwGetKey(window.window, GLFW_KEY_S)) {
        pos -= glm::vec3(dir.x, dir.y, dir.z) * static_cast<float>(delta) * speed;
    }
    if (glfwGetKey(window.window, GLFW_KEY_A)) {
        pos -= right * static_cast<float>(delta) * speed;
    }
    if (glfwGetKey(window.window, GLFW_KEY_D)) {
        pos += right * static_cast<float>(delta) * speed;
    }
    if (glfwGetKey(window.window, GLFW_KEY_E)) {
        pos += up * static_cast<float>(delta) * speed;
    }
    if (glfwGetKey(window.window, GLFW_KEY_Q)) {
        pos -= up * static_cast<float>(delta) * speed;
    }

    at = pos + glm::vec3(dir.x, dir.y, dir.z);

    prevMouseX = mouseX;
    prevMouseY = mouseY;
}