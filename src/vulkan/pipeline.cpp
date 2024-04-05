#include "pipeline.h"
#include "device.h"
#include "descriptor.h"
#include "shader.h"

namespace vkn {
void CreatePipeline()
{
    vk::GraphicsPipelineCreateInfo pipelineCI{};
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    meshRenderPipeline.shader.vertexShaderModule = Shader::CreateModule("shaders/base.vert.spv");
    meshRenderPipeline.shader.fragmentShaderModule = Shader::CreateModule("shaders/base.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, meshRenderPipeline.shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, meshRenderPipeline.shader.fragmentShaderModule, "main" };
    std::array<vk::DynamicState, 2> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
    attachmentStates[0] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[1] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[1].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;

    vk::VertexInputBindingDescription bindingDesc = MeshBase::GetBindingDesc();
    auto attributeDescs = MeshBase::GetAttributeDescs();
    vk::PipelineVertexInputStateCreateInfo vertexInputStateCI({}, 1, &bindingDesc, attributeDescs.size(), attributeDescs.data());
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCI({}, vk::PrimitiveTopology::eTriangleList);
    vk::PipelineDynamicStateCreateInfo dynamicStateCI({}, dynamicStates.size(), dynamicStates.data());
    vk::PipelineViewportStateCreateInfo viewportStateCI({}, 1, {}, 1, {});
    vk::PipelineRasterizationStateCreateInfo rasterizeStateCI({}, vk::False, vk::False, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False, {}, {}, {}, 1.0f);
    vk::PipelineMultisampleStateCreateInfo multisampleStateCI({}, vk::SampleCountFlagBits::e1, vk::False);
    vk::PipelineColorBlendStateCreateInfo colorBlendStateCI({}, vk::False, {}, attachmentStates.size(), attachmentStates.data());
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCI({}, vk::True, vk::True, vk::CompareOp::eLess);
    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(MeshRenderPushConstants));
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, meshRenderPipeline.descriptorSetLayouts.size(), meshRenderPipeline.descriptorSetLayouts.data(), 1, &pushConstantRange);

    pipelineCI.pVertexInputState = &vertexInputStateCI;
    pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    pipelineCI.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI.pStages = shaderStageInfos.data();
    pipelineCI.pDynamicState = &dynamicStateCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pRasterizationState = &rasterizeStateCI;
    pipelineCI.pMultisampleState = &multisampleStateCI;
    pipelineCI.pColorBlendState = &colorBlendStateCI;
    pipelineCI.pDepthStencilState = &depthStencilStateCI;
    pipelineCI.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfo);
    pipelineCI.renderPass = meshRenderPipeline.renderPass;

    meshRenderPipeline.pipelineLayout = pipelineCI.layout;
    meshRenderPipeline.pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI)).value;

    // Shadow Map
    pushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShadowMapPushConstants) };
    vk::PipelineColorBlendAttachmentState attachment;
    attachment = vk::PipelineColorBlendAttachmentState(vk::False);
    attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    rasterizeStateCI.cullMode = vk::CullModeFlagBits::eFront;
    pipelineLayoutInfo = { {}, (uint32_t)shadowMapPipeline.descriptorSetLayouts.size(), shadowMapPipeline.descriptorSetLayouts.data(), 1, &pushConstantRange };
    shadowMapPipeline.shader.vertexShaderModule = Shader::CreateModule("shaders/shadow.vert.spv");
    shadowMapPipeline.shader.fragmentShaderModule = Shader::CreateModule("shaders/shadow.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shadowMapPipeline.shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shadowMapPipeline.shader.fragmentShaderModule, "main" };
    colorBlendStateCI = { {}, vk::False, {}, 1, &attachment };
    pipelineCI.layout = Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfo);
    pipelineCI.renderPass = shadowMapPipeline.renderPass;

    shadowMapPipeline.pipelineLayout = pipelineCI.layout;
    shadowMapPipeline.pipeline = (Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI)).value;
}

void SetUpDescriptors()
{
    {
        // Mesh Render
        std::vector<vk::DescriptorPoolSize> poolSizes;
        uint32_t maxSets = 0;

        std::vector<vkn::DescriptorBinding> bindings;
        bindings = {
            // camera
            { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
            // light
            { 1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
            // mesh
            { 2, vk::DescriptorType::eStorageBuffer, 1000, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        };
        meshRenderPipeline.descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
        vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

        bindings = {
            // repeat sampler
            { 0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
            // texture array
            { 1, vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
            // cubemap
            { 2, vk::DescriptorType::eSampledImage, 1000, vk::ShaderStageFlagBits::eFragment, vk::DescriptorBindingFlagBits::ePartiallyBound },
        };
        meshRenderPipeline.descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
        vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

        // bindings = {
        //     // clamp sampler
        //     { 0, vk::descriptortype::esampler, 1, vk::shaderstageflagbits::efragment },
        //     // diffuse ibl
        //     { 1, vk::descriptortype::esampledimage, 1, vk::shaderstageflagbits::efragment },
        //     // specular ibl
        //     { 2, vk::descriptortype::esampledimage, 1, vk::shaderstageflagbits::efragment },
        //     // brdf
        //     { 3, vk::descriptortype::esampledimage, 1, vk::shaderstageflagbits::efragment },
        // };
        // meshrenderpipeline.descriptorsetlayouts.push_back(vkn::descriptor::createdescriptorsetlayout(bindings));
        // vkn::descriptorptor::setpoolsizes(poolsizes, bindings, maxsets);

        vkn::Descriptor::CreateDescriptorPool(meshRenderPipeline.descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
        vkn::Descriptor::AllocateDescriptorSets(meshRenderPipeline.descriptorPool, meshRenderPipeline.descriptorSets, meshRenderPipeline.descriptorSetLayouts);

        std::vector<vk::WriteDescriptorSet> write = {
            { meshRenderPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &meshRenderPipeline.cameraDescriptor },
            { meshRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &meshRenderPipeline.lightDescriptor },
        };
        vkn::Device::GetBundle().device.updateDescriptorSets(write, nullptr);
    }

    {
        // Shadow Map
        std::vector<vk::DescriptorPoolSize> poolSizes;
        uint32_t maxSets = 0;

        std::vector<vkn::DescriptorBinding> bindings;
        bindings = {
            // camera
            { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
            // lights
            { 1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound },
            // meshes
            { 2, vk::DescriptorType::eStorageBuffer, 1000, vk::ShaderStageFlagBits::eVertex, vk::DescriptorBindingFlagBits::ePartiallyBound }
        };
        shadowMapPipeline.descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings, vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool));
        vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

        vkn::Descriptor::CreateDescriptorPool(shadowMapPipeline.descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
        vkn::Descriptor::AllocateDescriptorSets(shadowMapPipeline.descriptorPool, shadowMapPipeline.descriptorSets, shadowMapPipeline.descriptorSetLayouts);

        std::vector<vk::WriteDescriptorSet> write = {
            { shadowMapPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &shadowMapPipeline.cameraDescriptor },
            { shadowMapPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &shadowMapPipeline.lightDescriptor },
        };
        vkn::Device::GetBundle().device.updateDescriptorSets(write, nullptr);
    }
}

void CreateMeshRenderPass()
{
    std::array<vk::AttachmentDescription, 3> meshRenderAttachments;

    meshRenderAttachments[0].format = vk::Format::eB8G8R8A8Srgb;
    meshRenderAttachments[0].samples = vk::SampleCountFlagBits::e1;
    meshRenderAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    meshRenderAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    meshRenderAttachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    meshRenderAttachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    meshRenderAttachments[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    meshRenderAttachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // ID
    meshRenderAttachments[1].format = vk::Format::eR32G32Sint;
    meshRenderAttachments[1].samples = vk::SampleCountFlagBits::e1;
    meshRenderAttachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    meshRenderAttachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    meshRenderAttachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    meshRenderAttachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    meshRenderAttachments[1].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    meshRenderAttachments[1].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // Depth
    meshRenderAttachments[2].format = vk::Format::eD32Sfloat;
    meshRenderAttachments[2].samples = vk::SampleCountFlagBits::e1;
    meshRenderAttachments[2].loadOp = vk::AttachmentLoadOp::eClear;
    meshRenderAttachments[2].storeOp = vk::AttachmentStoreOp::eStore;
    meshRenderAttachments[2].stencilLoadOp = vk::AttachmentLoadOp::eClear;
    meshRenderAttachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    meshRenderAttachments[2].initialLayout = vk::ImageLayout::eUndefined;
    meshRenderAttachments[2].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference idAttachmentRef;
    idAttachmentRef.attachment = 1;
    idAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    std::array<vk::AttachmentReference, 2> colorAttachmentRefs{ colorAttachmentRef, idAttachmentRef };

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = colorAttachmentRefs.size();
    subpassDesc.pColorAttachments = colorAttachmentRefs.data();
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = (uint32_t)(meshRenderAttachments.size());
    renderPassInfo.pAttachments = meshRenderAttachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    meshRenderPipeline.renderPass = vkn::Device::GetBundle().device.createRenderPass(renderPassInfo);
}

void CreateShadowMapRenderPass()
{
    std::array<vk::AttachmentDescription, 2> shadowMapAttachments;

    shadowMapAttachments[0].format = shadowMapImageFormat;
    shadowMapAttachments[0].samples = vk::SampleCountFlagBits::e1;
    shadowMapAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    shadowMapAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    shadowMapAttachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    shadowMapAttachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    shadowMapAttachments[0].initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    shadowMapAttachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Depth
    shadowMapAttachments[1].format = shadowMapDepthFormat;
    shadowMapAttachments[1].samples = vk::SampleCountFlagBits::e1;
    shadowMapAttachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    shadowMapAttachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    shadowMapAttachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    shadowMapAttachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    shadowMapAttachments[1].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    shadowMapAttachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    vk::RenderPassCreateInfo renderPassCI{};
    renderPassCI.attachmentCount = shadowMapAttachments.size();
    renderPassCI.pAttachments = shadowMapAttachments.data();
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDesc;

    vkn::CheckResult(vkn::Device::GetBundle().device.createRenderPass(&renderPassCI, nullptr, &shadowMapPipeline.renderPass));
}
} // namespace vkn
