#include "shader.h"

vk::ShaderModule Shader::CreateModule(std::string filepath)
{
    std::vector<char> sourceCode = ReadFile(filepath);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(sourceCode.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(sourceCode.data()));

    return Device::device.createShaderModule(createInfo);
}

Shader::~Shader()
{
    Device::device.destroyShaderModule(vertexShaderModule);
    Device::device.destroyShaderModule(fragmentShaderModule);
}

vk::ShaderModule Shader::vertexShaderModule;
vk::ShaderModule Shader::fragmentShaderModule;