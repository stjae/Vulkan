#include "shader.h"

vk::ShaderModule vkn::Shader::CreateModule(const std::string& filepath)
{
    std::vector<char> sourceCode = FetchCode(filepath);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(sourceCode.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(sourceCode.data()));

    return vkn::Device::Get().device.createShaderModule(createInfo);
}