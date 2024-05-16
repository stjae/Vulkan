#include "camera.h"

Camera::Camera()
{
    vkn::BufferInfo bufferInput = { sizeof(CameraUBO), sizeof(CameraUBO), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal };
    m_cameraBuffer = std::make_unique<vkn::Buffer>(bufferInput);
}

void Camera::SetCameraControl()
{
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        m_isControllable = !m_isControllable;
        ImGuiIO& io = ImGui::GetIO();

        if (m_isControllable) {
            glfwSetInputMode(Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        m_isInitial = true;
    }
}

void Camera::Update()
{
    SetCameraControl();

    if (!m_isControllable)
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

    if (m_isInitial) {
        prevMouseX = mouseX;
        prevMouseY = mouseY;
        m_isInitial = false;
    }

    glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), (float)(prevMouseX - mouseX), m_up);
    m_dir = rotateY * glm::vec4(m_dir, 0.0f);
    glm::normalize(m_dir);
    m_right = glm::cross(m_dir, m_up);
    glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), (float)(prevMouseY - mouseY), m_right);
    m_dir = rotateX * glm::vec4(m_dir, 0.0f);

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        m_pos += m_dir * (float)delta * m_speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        m_pos -= m_dir * (float)delta * m_speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        m_pos -= m_right * (float)delta * m_speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        m_pos += m_right * (float)delta * m_speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_E)) {
        m_pos += m_up * (float)delta * m_speed;
    }
    if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        m_pos -= m_up * (float)delta * m_speed;
    }

    m_at = m_pos + m_dir;

    prevMouseX = mouseX;
    prevMouseY = mouseY;
}