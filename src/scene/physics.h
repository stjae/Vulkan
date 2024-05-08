#ifndef PHYSICS_H
#define PHYSICS_H

#include <bullet/btBulletCollisionCommon.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include "../pipeline/lineRender.h"
#include "../scene/mesh/meshModel.h"

class DebugDrawer : public btIDebugDraw
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
        vkn::BufferInput bufferInput = { sizeof(LinePoint) * m_linePoints.size(), sizeof(LinePoint), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
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

    void CopyBuffer()
    {
        vkn::Command::Begin(m_commandBuffer);
        vkn::Command::CopyBufferToBuffer(m_commandBuffer,
                                         m_vertexStagingBuffer->GetBundle().buffer,
                                         m_vertexBuffer->GetBundle().buffer,
                                         m_vertexStagingBuffer->GetBufferInput().size);
        vkn::Command::CopyBufferToBuffer(m_commandBuffer,
                                         m_indexStagingBuffer->GetBundle().buffer,
                                         m_indexBuffer->GetBundle().buffer,
                                         m_indexStagingBuffer->GetBufferInput().size);
        m_commandBuffer.end();
        vkn::Command::Submit(&m_commandBuffer, 1);
    }

    ~DebugDrawer() override
    {
        vkn::Device::GetBundle().device.destroyCommandPool(m_commandPool);
    }
};

class Physics
{
    friend class Viewport;

    btDefaultCollisionConfiguration* collisionConfiguration_;
    btCollisionDispatcher* dispatcher_;
    btBroadphaseInterface* overlappingPairCache_;
    btSequentialImpulseConstraintSolver* solver_;
    btDiscreteDynamicsWorld* dynamicsWorld_;

    btAlignedObjectArray<btCollisionShape*> collisionShapes_;

    DebugDrawer debugDrawer_;

    void DebugDraw()
    {
        dynamicsWorld_->debugDrawWorld();

        if (!debugDrawer_.m_linePoints.empty()) {
            debugDrawer_.CreateBuffer();
            debugDrawer_.CopyBuffer();
        }
    }

public:
    void InitPhysics();
    void AddRigidBody(MeshInstanceUBO& ubo, const MeshInstancePhysicsInfo& pInfo);
    void Simulate(std::vector<MeshModel>& meshes);
    void Stop(std::vector<MeshModel>& meshes);
    void Update(Mesh& mesh);
    void DeleteRigidBody(MeshInstanceUBO& ubo);
    void UpdateRigidBody(const MeshInstanceUBO& ubo);
    ~Physics();
};

#endif