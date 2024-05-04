#include "lineRender.h"

void CreateRenderPass();
LineRenderPipeline::LineRenderPipeline()
{
    bindingDesc_.setBinding(0);
    bindingDesc_.setStride(sizeof(LinePoint));
    bindingDesc_.setInputRate(vk::VertexInputRate::eVertex);

    uint32_t offset = 0;
    // Pos
    vertexInputAttribDesc_[0].binding = 0;
    vertexInputAttribDesc_[0].location = 0;
    vertexInputAttribDesc_[0].format = vk::Format::eR32G32B32Sfloat;
    vertexInputAttribDesc_[0].offset = offset;
    offset += sizeof(LinePoint::pos);
    // Color
    vertexInputAttribDesc_[1].binding = 0;
    vertexInputAttribDesc_[1].location = 1;
    vertexInputAttribDesc_[1].format = vk::Format::eR32G32B32Sfloat;
    vertexInputAttribDesc_[1].offset = offset;

    vertexInputStateCI_ = { {}, 1, &bindingDesc_, (uint32_t)vertexInputAttribDesc_.size(), vertexInputAttribDesc_.data() };
    inputAssemblyStateCI_ = { {}, vk::PrimitiveTopology::eLineList };
    rasterizeStateCI_ = { {}, vk::False, vk::False, vk::PolygonMode::eLine, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, vk::False, {}, {}, {}, 1.0f };

    pipelineCI_.pVertexInputState = &vertexInputStateCI_;
    pipelineCI_.pInputAssemblyState = &inputAssemblyStateCI_;
    pipelineCI_.pRasterizationState = &rasterizeStateCI_;
}

void LineRenderPipeline::CreatePipeline()
{
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStageInfos;
    std::array<vk::PipelineColorBlendAttachmentState, 2> attachmentStates;
    shader.vertexShaderModule = vkn::Shader::CreateModule("shaders/line.vert.spv");
    shader.fragmentShaderModule = vkn::Shader::CreateModule("shaders/line.frag.spv");
    shaderStageInfos[0] = { {}, vk::ShaderStageFlagBits::eVertex, shader.vertexShaderModule, "main" };
    shaderStageInfos[1] = { {}, vk::ShaderStageFlagBits::eFragment, shader.fragmentShaderModule, "main" };
    // attachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[0] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    attachmentStates[1] = vk::PipelineColorBlendAttachmentState(vk::False);
    attachmentStates[1].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG;

    SetUpDescriptors();
    CreateRenderPass();

    vk::PipelineLayoutCreateInfo pipelineLayoutInfoCI({}, descriptorSetLayouts.size(), descriptorSetLayouts.data(), 0, nullptr);

    pipelineCI_.stageCount = (uint32_t)shaderStageInfos.size();
    pipelineCI_.pStages = shaderStageInfos.data();
    colorBlendStateCI_ = { {}, vk::False, {}, attachmentStates.size(), attachmentStates.data() };
    pipelineCI_.pColorBlendState = &colorBlendStateCI_;
    pipelineCI_.layout = vkn::Device::GetBundle().device.createPipelineLayout(pipelineLayoutInfoCI);
    pipelineCI_.renderPass = renderPass;

    pipelineLayout = pipelineCI_.layout;
    pipeline = (vkn::Device::GetBundle().device.createGraphicsPipeline(nullptr, pipelineCI_)).value;
}

void LineRenderPipeline::SetUpDescriptors()
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> bindings;
    bindings = {
        // camera
        { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
    };
    descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(bindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, bindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool, poolSizes, maxSets);
    vkn::Descriptor::AllocateDescriptorSets(descriptorPool, descriptorSets, descriptorSetLayouts);
}

void LineRenderPipeline::CreateRenderPass()
{
    // std::array<vk::AttachmentDescription, 2> meshRenderAttachments;
    //
    // meshRenderAttachments[0].format = vk::Format::eB8G8R8A8Srgb;
    // meshRenderAttachments[0].samples = vk::SampleCountFlagBits::e1;
    // meshRenderAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    // meshRenderAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    // meshRenderAttachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    // meshRenderAttachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    // meshRenderAttachments[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    // meshRenderAttachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
    //
    // // Depth
    // meshRenderAttachments[1].format = vk::Format::eD32Sfloat;
    // meshRenderAttachments[1].samples = vk::SampleCountFlagBits::e1;
    // meshRenderAttachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    // meshRenderAttachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    // meshRenderAttachments[1].stencilLoadOp = vk::AttachmentLoadOp::eClear;
    // meshRenderAttachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    // meshRenderAttachments[1].initialLayout = vk::ImageLayout::eUndefined;
    // meshRenderAttachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    //
    // vk::AttachmentReference colorAttachmentRef;
    // colorAttachmentRef.attachment = 0;
    // colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
    //
    // vk::AttachmentReference depthAttachmentRef;
    // depthAttachmentRef.attachment = 1;
    // depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    //
    // vk::SubpassDescription subpassDesc;
    // subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    // subpassDesc.colorAttachmentCount = 1;
    // subpassDesc.pColorAttachments = &colorAttachmentRef;
    // subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;
    //
    // vk::RenderPassCreateInfo renderPassInfo;
    // renderPassInfo.attachmentCount = (uint32_t)(meshRenderAttachments.size());
    // renderPassInfo.pAttachments = meshRenderAttachments.data();
    // renderPassInfo.subpassCount = 1;
    // renderPassInfo.pSubpasses = &subpassDesc;
    //
    // renderPass = vkn::Device::GetBundle().device.createRenderPass(renderPassInfo);
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

    renderPass = vkn::Device::GetBundle().device.createRenderPass(renderPassInfo);
}

void LineRenderPipeline::UpdateCameraDescriptor()
{
    vk::WriteDescriptorSet writeDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSets[0];
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo = &cameraDescriptor;
    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSet, nullptr);
}
