#ifndef SHADER_H
#define SHADER_H

#include "../common.h"
#include "device.h"

class Shader
{
public:
    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
    vk::ShaderModule computeShaderModule;

    vk::ShaderModule CreateModule(const std::string& filepath);

    ~Shader();
};

#endif