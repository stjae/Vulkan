#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "../vulkan/sync.h"
#include "../vulkan/image.h"
#include "../scene/mesh.h"
#include "../pipeline/shadowMap.h"

const uint16_t SHADOW_MAP_SIZE = 4096;

class ShadowMap : public vkn::Image
{
    friend class UI;
    friend class SceneSerializer;

    ShadowMapPushConstants m_pushConstants = {};

    std::unique_ptr<vkn::Buffer> m_viewProjBuffer;
    glm::mat4 m_viewProj = {};
    float m_nearPlane = 1.0f;
    float m_farPlane = 45.0f;
    float m_size = 10.0f;

public:
    ShadowMap();
    void CreateShadowMap(vk::CommandBuffer& commandBuffer);
    void DrawShadowMap(vk::CommandBuffer& commandBuffer, std::vector<std::shared_ptr<Mesh>>& meshes);
    void Update(const glm::vec3& ligthPos);
};

#endif