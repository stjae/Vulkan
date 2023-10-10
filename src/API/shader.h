#ifndef _SHADER_H_
#define _SHADER_H_

#include "../common.h"
#include "config.h"
#include "device.h"

class Shader
{
public:
    ~Shader();
    vk::ShaderModule CreateModule(std::string filepath);

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;
};

#endif