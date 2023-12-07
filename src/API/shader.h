#ifndef _SHADER_H_
#define _SHADER_H_

#include "../common.h"
#include "device.h"

class Shader
{
public:
    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;

    vk::ShaderModule CreateModule(std::string filepath);

    ~Shader();
};

#endif