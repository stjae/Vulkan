#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <memory>
#include "../vulkan/buffer.h"
#include "../pipeline/meshRender.h"
#include "../pipeline/shadowCubemap.h"
#include "../vulkan/command.h"
#include "../vulkan/sync.h"

struct DirLightUBO
{
    glm::vec3 dir;
    float intensity = 1.0f;
    glm::vec3 color = glm::vec3(1.0f);
};

class DirLight
{
    friend class SceneSerializer;
    friend class UI;

    DirLightUBO m_UBO;
    std::unique_ptr<vkn::Buffer> m_dirLightUBOBuffer;
    glm::vec3 m_position = glm::vec3(0.1f, 10.0f, 0.1f);

public:
    DirLight();
    const glm::vec3& GetPosition() { return m_position; }
    void SetPosition(const glm::vec3& position) { m_position = position; }
    void Update();
};

struct PointLightUBO
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float padding = 0.0f;
};

class PointLight
{
    friend class SceneSerializer;
    friend class UI;

    std::unique_ptr<vkn::Buffer> m_UBOBuffer;
    std::unique_ptr<vkn::Buffer> m_UBOStagingBuffer;
    std::vector<PointLightUBO> m_UBOs;

public:
    void Create();
    void Add() { m_UBOs.emplace_back(); }
    void Duplicate(int index);
    void Delete(int index) { m_UBOs.erase(m_UBOs.begin() + index); }
    void Clear() { m_UBOs.clear(); }
    size_t Size() const { return m_UBOs.size(); }
    const std::vector<PointLightUBO>& Get() const { return m_UBOs; }
    void Update(const vk::CommandBuffer& commandBuffer);
    void UpdateBuffer();
};

#endif
