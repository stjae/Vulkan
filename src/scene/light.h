// Creation and update of directional and point lights
// ディレクショナルライトとポイントライトの生成と更新

#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <memory>
#include "../vulkan/buffer.h"
#include "../pipeline/meshRenderPipeline.h"
#include "../pipeline/shadowCubemapPipeline.h"
#include "../vulkan/command.h"
#include "../vulkan/sync.h"

struct DirLight
{
    glm::vec3 pos{ 0.1, 10.0f, 0.1f };
    float intensity = 1.0f;
    glm::vec3 color{ 1.0f };

    void Init();
};

struct PointLightUBO
{
    glm::vec3 pos{ 0.0f };
    float intensity = 5.0f;
    glm::vec3 color{ 1.0f };
    float range = 10.0f;
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
    const std::vector<PointLightUBO>& GetUBOs() const { return m_UBOs; }
    void Update(const vk::CommandBuffer& commandBuffer);
    void UpdateBuffer();
};

#endif
