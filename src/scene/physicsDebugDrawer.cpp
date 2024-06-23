#include "physicsDebugDrawer.h"

PhysicsDebugDrawer::PhysicsDebugDrawer(const vk::CommandBuffer& commandBuffer, const PhysicsInfo& physicsInfo, const std::vector<std::vector<uint32_t>>& indexContainers, const std::vector<std::vector<Vertex>>& vertexContainers)
{
    btVector3 color = physicsInfo.rigidBodyType == eRigidBodyType::STATIC ? btVector3(0.5f, 0.0f, 0.0f) : btVector3(0.0f, 0.5f, 0.0f);
    btTransform transform;
    transform.setIdentity();
    switch (physicsInfo.colliderShape) {
    case (eColliderShape::BOX):
        drawBox(btVector3(-0.5f, -0.5f, -0.5f), btVector3(0.5f, 0.5f, 0.5f), color);
        break;
    case (eColliderShape::SPHERE):
        drawSphere(btVector3(0.0f, 0.0f, 0.0f), 1.03f, color);
        break;
    case (eColliderShape::CAPSULE):
        drawCapsule(1.0f, 0.5f, 1, transform, color);
        break;
    case (eColliderShape::CYLINDER):
        drawCylinder(1.0f, 0.5f, 1, transform, color);
        break;
    case (eColliderShape::CONE):
        drawCone(1.0f, 1.0f, 1, transform, color);
        break;
    case (eColliderShape::MESH):
        DrawMesh(indexContainers, vertexContainers, color);
        break;
    default:
        return;
    }

    CreateBuffer();
    CopyBuffer(commandBuffer);
}

void PhysicsDebugDrawer::DrawMesh(const std::vector<std::vector<uint32_t>>& indexContainers, const std::vector<std::vector<Vertex>>& vertexContainers, const btVector3& color)
{
    for (int i = 0; i < indexContainers.size(); i++) {
        auto& indexContainer = indexContainers[i];
        auto& vertexContainer = vertexContainers[i];

        for (int j = 0; j < indexContainer.size(); j += 3) {
            auto& pos0 = vertexContainer[indexContainer[j]].pos;
            auto& pos1 = vertexContainer[indexContainer[j + 1]].pos;
            auto& pos2 = vertexContainer[indexContainer[j + 2]].pos;
            btVector3 v0(pos0.x, pos0.y, pos0.z);
            btVector3 v1(pos1.x, pos1.y, pos1.z);
            btVector3 v2(pos2.x, pos2.y, pos2.z);

            drawTriangle(v0, v1, v2, color, 1.0f);
        }
    }
}

void PhysicsDebugDrawer::setDebugMode(int debugMode)
{
    m_debugMode = debugMode;
}

int PhysicsDebugDrawer::getDebugMode() const
{
    return m_debugMode;
}

void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    m_linePoints.emplace_back(from, color);
    m_linePoints.emplace_back(to, color);

    m_lineIndices.push_back(m_lineIndices.size());
    m_lineIndices.push_back(m_lineIndices.size());
}

void PhysicsDebugDrawer::clearLines()
{
    m_linePoints.clear();
    m_lineIndices.clear();
}

void PhysicsDebugDrawer::CreateBuffer()
{
    vkn::BufferInfo bufferInfo = { sizeof(LinePoint) * m_linePoints.size(), sizeof(LinePoint), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_vertexStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    m_vertexStagingBuffer->Copy(m_linePoints.data());

    bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    bufferInfo.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    m_vertexBuffer = std::make_unique<vkn::Buffer>(bufferInfo);

    bufferInfo = { sizeof(uint32_t) * m_lineIndices.size(), sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_indexStagingBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    m_indexStagingBuffer->Copy(m_lineIndices.data());

    bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    bufferInfo.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    m_indexBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
}

void PhysicsDebugDrawer::CopyBuffer(const vk::CommandBuffer& commandBuffer) const
{
    vkn::Command::CopyBufferToBuffer(commandBuffer, m_vertexStagingBuffer->Get().buffer, m_vertexBuffer->Get().buffer, m_vertexStagingBuffer->Get().bufferInfo.size);
    vkn::Command::CopyBufferToBuffer(commandBuffer, m_indexStagingBuffer->Get().buffer, m_indexBuffer->Get().buffer, m_indexStagingBuffer->Get().bufferInfo.size);
}
