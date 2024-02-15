#ifndef LIGHT_H
#define LIGHT_H

#include "../common.h"
#include "../API/buffer.h"

struct LightUniformData
{
    glm::mat4 modelMat;
    glm::vec3 pos;
};

class Light
{
    friend class Scene;
    friend class MyImGui;

    LightUniformData lightUniformData;
    std::unique_ptr<Buffer> uniformBuffer_;

    void UpdateBuffer() { uniformBuffer_->UpdateBuffer(&lightUniformData, sizeof(LightUniformData)); }

public:
    Light();
    const vk::DescriptorBufferInfo& GetBufferInfo() { return uniformBuffer_->GetBundle().bufferInfo; }
};

#endif