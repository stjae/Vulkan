#ifndef _SHADER_H_
#define _SHADER_H_

#include "../common.h"
#include "device.h"

class Shader
{
public:
    ~Shader();
    vk::ShaderModule CreateModule(std::string filepath);

    vk::ShaderModule m_vertexShaderModule;
    vk::ShaderModule m_fragmentShaderModule;
};

#endif