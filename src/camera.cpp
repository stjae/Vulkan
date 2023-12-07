#include "camera.h"

void Camera::Update()
{
    static double delta, currentTime, lastTime = 0.0;
    currentTime = ImGui::GetTime();

    delta = currentTime - lastTime;
    lastTime = currentTime;

    static float prevMouseX, prevMouseY;
    static float mouseX, mouseY;

    ImGuiIO& io = ImGui::GetIO();
    mouseX = io.MousePos.x;
    mouseY = io.MousePos.y;

    int width, height;
    glfwGetWindowSize(*Window::GetWindow(), &width, &height);
    mouseX /= width;
    mouseY /= height;
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

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        pos += glm::vec3(dir.x, dir.y, dir.z) * static_cast<float>(delta) * speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        pos -= glm::vec3(dir.x, dir.y, dir.z) * static_cast<float>(delta) * speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        pos -= right * static_cast<float>(delta) * speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        pos += right * static_cast<float>(delta) * speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_E)) {
        pos += up * static_cast<float>(delta) * speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        pos -= up * static_cast<float>(delta) * speed;
    }

    at = pos + glm::vec3(dir.x, dir.y, dir.z);

    prevMouseX = mouseX;
    prevMouseY = mouseY;
}