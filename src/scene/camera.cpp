#include "camera.h"

Camera::Camera()
{
    BufferInput input = { sizeof(CameraMatrix), vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible |
                              vk::MemoryPropertyFlagBits::eHostCoherent };
    uniformBuffer_ = std::make_unique<Buffer>(input);
    uniformBuffer_->MapMemory(input.size);
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

    glm::mat4 rotateX =
        glm::rotate(glm::mat4(1.0f), static_cast<float>(prevMouseX - mouseX), up_);
    dir_ = rotateX * dir_;
    glm::normalize(dir_);
    right_ = glm::cross(glm::vec3(dir_.x, dir_.y, dir_.z), up_);
    glm::mat4 rotateY =
        glm::rotate(glm::mat4(1.0f), static_cast<float>(prevMouseY - mouseY), right_);
    dir_ = rotateY * dir_;

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        pos_ += glm::vec3(dir_.x, dir_.y, dir_.z) * static_cast<float>(delta) * speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        pos_ -= glm::vec3(dir_.x, dir_.y, dir_.z) * static_cast<float>(delta) * speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        pos_ -= right_ * static_cast<float>(delta) * speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        pos_ += right_ * static_cast<float>(delta) * speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_E)) {
        pos_ += up_ * static_cast<float>(delta) * speed_;
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        pos_ -= up_ * static_cast<float>(delta) * speed_;
    }

    at_ = pos_ + glm::vec3(dir_.x, dir_.y, dir_.z);

    prevMouseX = mouseX;
    prevMouseY = mouseY;
}

void Camera::UpdateBuffer()
{
    uniformBuffer_->UpdateBuffer(&matrix_, sizeof(CameraMatrix));
}

const vk::DescriptorBufferInfo& Camera::GetBufferInfo()
{
    return uniformBuffer_->GetBundle().bufferInfo;
}
