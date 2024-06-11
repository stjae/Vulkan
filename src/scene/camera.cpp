#include "camera.h"
#include "../vulkan/swapchain.h"
#include "mesh.h"

Camera::Camera()
{
    vkn::BufferInfo bufferInput = { sizeof(CameraUBO), sizeof(CameraUBO), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_cameraStagingBuffer = std::make_unique<vkn::Buffer>(bufferInput);
    bufferInput = { sizeof(CameraUBO), sizeof(CameraUBO), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal };
    m_cameraBuffer = std::make_unique<vkn::Buffer>(bufferInput);
}

void Camera::Update(const vk::CommandBuffer& commandBuffer)
{
    m_cameraUBO.view = glm::lookAt(m_pos, m_at, m_up);
    m_cameraUBO.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height), m_zNear, m_zFar);
    m_cameraUBO.pos = m_pos;
    m_cameraStagingBuffer->Copy(&m_cameraUBO);

    vkn::Command::CopyBufferToBuffer(commandBuffer, m_cameraStagingBuffer->Get().buffer, m_cameraBuffer->Get().buffer, m_cameraStagingBuffer->Get().bufferInfo.size);
}

void Camera::SetControl()
{
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        m_isControllable = !m_isControllable;

        if (m_isControllable) {
            glfwSetInputMode(Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        m_startControl = true;
    }
}

float Camera::GetCascadeDepth(int index)
{
    return m_cascadeRanges[index];
}

glm::mat4 Camera::GetCascadeProj(int index)
{
    switch (index) {
    case 0:
        return glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height), m_zNear, m_cascadeRanges[0]);
    case 1:
        return glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height), m_cascadeRanges[0], m_cascadeRanges[1]);
    case 2:
        return glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height), m_cascadeRanges[1], m_cascadeRanges[2]);
    default:
        return glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height), m_cascadeRanges[2], m_cascadeRanges[3]);
    }
}

uint64_t Camera::GetAssignedMeshInstanceID()
{
    return m_assignedMeshInstanceID;
}

void MainCamera::Control()
{
    SetControl();

    if (m_isControllable) {
        static float prevMouseX, prevMouseY;
        static float mouseX, mouseY;

        mouseX = Window::GetMousePosNormalizedX();
        mouseY = Window::GetMousePosNormalizedY();

        if (m_startControl) {
            prevMouseX = mouseX;
            prevMouseY = mouseY;
            m_startControl = false;
        }

        glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), (float)(prevMouseX - mouseX), m_up);
        m_dir = rotateY * glm::vec4(m_dir, 0.0f);
        glm::normalize(m_dir);
        m_right = glm::cross(m_dir, m_up);
        glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), (float)(prevMouseY - mouseY), m_right);
        m_dir = rotateX * glm::vec4(m_dir, 0.0f);

        float dt = Time::GetDeltaTime();

        if (ImGui::IsKeyDown(ImGuiKey_W)) {
            m_pos += m_dir * (float)dt * m_speed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_S)) {
            m_pos -= m_dir * (float)dt * m_speed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_A)) {
            m_pos -= m_right * (float)dt * m_speed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_D)) {
            m_pos += m_right * (float)dt * m_speed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_E)) {
            m_pos += m_up * (float)dt * m_speed;
        }
        if (ImGui::IsKeyDown(ImGuiKey_Q)) {
            m_pos -= m_up * (float)dt * m_speed;
        }

        m_at = m_pos + m_dir;

        prevMouseX = mouseX;
        prevMouseY = mouseY;
    }
}

SubCamera::SubCamera(uint64_t meshInstanceID)
{
    m_assignedMeshInstanceID = meshInstanceID;
}

void SubCamera::Control()
{
    if (m_isFirstFrame) {
        m_isControllable = true;
        glfwSetInputMode(Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;

        m_isFirstFrame = false;
    }
    SetControl();

    // m_pos = glm::make_vec3(translation);
    // m_dir = glm::make_mat4(outMatrix) * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    // m_at = m_dir + m_pos;
}