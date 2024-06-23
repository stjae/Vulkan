#ifndef PHYSICSDEBUGDRAWER_H
#define PHYSICSDEBUGDRAWER_H

#include <btBulletCollisionCommon.h>
#include "../vulkan/command.h"
#include "../enum.h"
#include "meshBase.h"
#include "line.h"

struct PhysicsInfo
{
    eRigidBodyType rigidBodyType;
    eColliderShape colliderShape;
    btRigidBody* rigidBodyPtr;
    btCollisionShape* collisionShapePtr;
    glm::vec3 scale = glm::vec3(1.0f);
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

    PhysicsDebugDrawer(const vk::CommandBuffer& commandBuffer, const PhysicsInfo& physicsInfo, const std::vector<std::vector<uint32_t>>& indexContainers, const std::vector<std::vector<Vertex>>& vertexContainers);
    void DrawMesh(const std::vector<std::vector<uint32_t>>& indexContainers, const std::vector<std::vector<Vertex>>& vertexContainers, const btVector3& color);
    void setDebugMode(int debugMode) override;
    int getDebugMode() const override;
    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
    void reportErrorWarning(const char* warningString) override {}
    void draw3dText(const btVector3& location, const char* textString) override {}
    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override {}
    void clearLines() override;
    void CreateBuffer();
    void CopyBuffer(const vk::CommandBuffer& commandBuffer) const;
};

#endif