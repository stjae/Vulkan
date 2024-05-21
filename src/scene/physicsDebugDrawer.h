#ifndef PHYSICSDEBUGDRAWER_H
#define PHYSICSDEBUGDRAWER_H

#include <bullet/btBulletCollisionCommon.h>
#include "../vulkan/command.h"
// #include "../struct.h"
#include "../enum.h"
#include "meshBase.h"

struct PhysicsInfo
{
    eRigidBodyType rigidBodyType;
    eColliderShape colliderShape;
    btRigidBody* rigidBodyPtr;
    // model matrix before simulation
    glm::mat4 initialModel;
    glm::vec3 scale;

    PhysicsInfo() : scale(1.0f) {}
};

struct LinePoint
{
    glm::vec3 pos;
    glm::vec3 color;
    LinePoint(const btVector3& pos, const btVector3& color) : pos(pos.x(), pos.y(), pos.z()), color(color.x(), color.y(), color.z()) {}
};

class PhysicsDebugDrawer : public btIDebugDraw
{
public:
    int m_debugMode;
    std::vector<LinePoint> m_linePoints;
    std::vector<uint32_t> m_lineIndices;

    std::unique_ptr<vkn::Buffer> m_vertexStagingBuffer;
    std::unique_ptr<vkn::Buffer> m_vertexBuffer;
    std::unique_ptr<vkn::Buffer> m_indexStagingBuffer;
    std::unique_ptr<vkn::Buffer> m_indexBuffer;

    vk::CommandPool m_commandPool;
    vk::CommandBuffer m_commandBuffer;

    void DrawMesh(const std::vector<std::vector<uint32_t>>& indexContainers, const std::vector<std::vector<Vertex>>& vertexContainers, const btVector3& color)
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

    PhysicsDebugDrawer(const PhysicsInfo& physicsInfo, const std::vector<std::vector<uint32_t>>& indexContainers, const std::vector<std::vector<Vertex>>& vertexContainers)
    {
        vkn::Command::CreateCommandPool(m_commandPool);
        vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffer);

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
            drawCapsule(1.0f, 1.0f, 1, transform, color);
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
        CopyBuffer();
    }

    void setDebugMode(int debugMode) override
    {
        m_debugMode = debugMode;
    }

    int getDebugMode() const override
    {
        return m_debugMode;
    }

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
    {
        m_linePoints.emplace_back(from, color);
        m_linePoints.emplace_back(to, color);

        m_lineIndices.push_back(m_lineIndices.size());
        m_lineIndices.push_back(m_lineIndices.size());
    }

    void reportErrorWarning(const char* warningString) override
    {
    }

    void draw3dText(const btVector3& location, const char* textString) override
    {
    }

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override
    {
    }

    void clearLines() override
    {
        m_linePoints.clear();
        m_lineIndices.clear();
    }

    void CreateBuffer()
    {
        vkn::BufferInfo bufferInput = { sizeof(LinePoint) * m_linePoints.size(), sizeof(LinePoint), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_vertexStagingBuffer = std::make_unique<vkn::Buffer>(bufferInput);
        m_vertexStagingBuffer->Copy(m_linePoints.data());

        bufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
        bufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
        m_vertexBuffer = std::make_unique<vkn::Buffer>(bufferInput);

        bufferInput = { sizeof(uint32_t) * m_lineIndices.size(), sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_indexStagingBuffer = std::make_unique<vkn::Buffer>(bufferInput);
        m_indexStagingBuffer->Copy(m_lineIndices.data());

        bufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
        bufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
        m_indexBuffer = std::make_unique<vkn::Buffer>(bufferInput);
    }

    void CopyBuffer() const
    {
        vkn::Command::Begin(m_commandBuffer);
        vkn::Command::CopyBufferToBuffer(m_commandBuffer,
                                         m_vertexStagingBuffer->Get().buffer,
                                         m_vertexBuffer->Get().buffer,
                                         m_vertexStagingBuffer->Get().bufferInfo.size);
        vkn::Command::CopyBufferToBuffer(m_commandBuffer,
                                         m_indexStagingBuffer->Get().buffer,
                                         m_indexBuffer->Get().buffer,
                                         m_indexStagingBuffer->Get().bufferInfo.size);
        m_commandBuffer.end();

        vkn::Device::s_submitInfos.emplace_back(0, nullptr, nullptr, 1, &m_commandBuffer);
    }

    ~PhysicsDebugDrawer() override
    {
        vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    }
};

#endif