#include "shader.h"

vk::ShaderModule Shader::CreateModule(const std::string& filepath)
{
    std::vector<char> sourceCode = ReadCode(filepath);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(sourceCode.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(sourceCode.data()));

    return Device::GetBundle().device.createShaderModule(createInfo);
}