#include "camera.h"

Camera::Camera(const vk::CommandPool& commandPool)
{
    vkn::Command::AllocateCommandBuffer(commandPool, m_commandBuffers);
    vkn::BufferInfo bufferInput = { sizeof(CameraUBO), sizeof(CameraUBO), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_cameraStagingBuffer = std::make_unique<vkn::Buffer>(bufferInput);
    bufferInput = { sizeof(CameraUBO), sizeof(CameraUBO), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal };
    m_cameraBuffer = std::make_unique<vkn::Buffer>(bufferInput);
}

void Camera::Update()
{
    m_cameraUBO.view = glm::lookAt(m_pos, m_at, m_up);
    m_cameraUBO.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height), 0.1f, 1024.0f);
    m_cameraUBO.pos = m_pos;
    m_cameraStagingBuffer->Copy(&m_cameraUBO);

    vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    vkn::Command::CopyBufferToBuffer(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], m_cameraStagingBuffer->Get().buffer, m_cameraBuffer->Get().buffer, m_cameraStagingBuffer->Get().bufferInfo.size);
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
    vkn::Device::s_submitInfos.emplace_back(0, nullptr, nullptr, 1, &m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
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

void MainCamera::Control()
{
    SetControl();

    if (m_isControllable) {
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

void SubCamera::ControlByMatrix(const glm::mat4& matrix)
{
    if (m_isFirstFrame) {
        m_isControllable = true;
        glfwSetInputMode(Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;

        m_isFirstFrame = false;
    }

    SetControl();

    m_pos = glm::vec3(matrix[3].x, matrix[3].y, matrix[3].z);

    if (m_isControllable) {
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

        m_at = m_pos + m_dir;

        prevMouseX = mouseX;
        prevMouseY = mouseY;
    }
}