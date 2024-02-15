#include "light.h"

Light::Light()
{
    BufferInput input = { sizeof(LightUniformData), vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible |
                              vk::MemoryPropertyFlagBits::eHostCoherent };
    uniformBuffer_ = std::make_unique<Buffer>(input);
    uniformBuffer_->MapMemory(input.size);
}
