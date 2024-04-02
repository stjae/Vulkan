#ifndef SHADER_H
#define SHADER_H

#include "../common.h"
#include "device.h"

namespace vkn {
class Shader
{
public:
    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;

    static vk::ShaderModule CreateModule(const std::string& filepath);
};
} // namespace vkn

#endif