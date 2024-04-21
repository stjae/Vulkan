#include "camera.h"

Camera::Camera()
{
    vkn::BufferInput bufferInput = { sizeof(CameraUBO), sizeof(CameraUBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    cameraBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
}

void Camera::SetCameraControl()
{
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        isControllable_ = !isControllable_;
        ImGuiIO& io = ImGui::GetIO();

        if (isControllable_) {
            glfwSetInputMode(Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        isInitial_ = true;
    }
}

void Camera::Update()
{
    SetCameraControl();

    if (!isControllable_)
        return;

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
    glfwGetWindowSize(Window::GetWindow(), &width, &height);
    mouseX /= (float)width;
    mouseY /= (float)height;
    mouseX -= 0.5f;
    mouseY -= 0.5f;

    if (isInitial_) {
        prevMouseX = mouseX;
        prevMouseY = mouseY;
        isInitial_ = false;
    }

    glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), (float)(prevMouseX - mouseX), up_);
    dir_ = rotateY * glm::vec4(dir_, 0.0f);
    glm::normalize(dir_);
    right_ = glm::cross(dir_, up_);
    glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), (float)(prevMouseY - mouseY), right_);
    dir_ = rotateX * glm::vec4(dir_, 0.0f);

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        pos_ += dir_ * (float)(delta)*speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        pos_ -= dir_ * (float)(delta)*speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        pos_ -= right_ * (float)(delta)*speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        pos_ += right_ * (float)(delta)*speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_E)) {
        pos_ += up_ * (float)(delta)*speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        pos_ -= up_ * (float)(delta)*speed_;
    }

    at_ = pos_ + dir_;

    prevMouseX = mouseX;
    prevMouseY = mouseY;
}