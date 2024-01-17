#include "viewport.h"

Viewport::Viewport()
{
    frames_.resize(Swapchain::Get().frameCount);

    CreateDescriptorSetLayout();
    CreateRenderPass();
    pipeline_.CreatePipeline(renderPass_, descriptorSetLayouts_);

    for (auto& frame : frames_) {

        frame.command.CreateCommandPool();
        frame.command.AllocateCommandBuffer();

        frame.descriptor.CreateDescriptorPool(1, descriptorSetLayoutData_, descriptorSetLayouts_.size(), descriptorPoolCreateFlags_);
        frame.descriptor.AllocateDescriptorSets(descriptorSetLayouts_);

        frame.viewportImage.CreateImage(vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eHostVisible, { Swapchain::Get().swapchainImageExtent.width, Swapchain::Get().swapchainImageExtent.height, 1 });
        frame.viewportImage.CreateImageView(vk::Format::eB8G8R8A8Srgb, vk::ImageAspectFlagBits::eColor);
        frame.viewportImage.CreateSampler();
        frame.viewportImage.SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);

        frame.depthImage.CreateImage(vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, { Swapchain::Get().swapchainImageExtent.width, Swapchain::Get().swapchainImageExtent.height, 1 });
        frame.depthImage.CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);
    }

    CreateFrameBuffer();
}

void Viewport::CreateDescriptorSetLayout()
{
    DescriptorSetLayoutData layout0;
    layout0.descriptorSetCount = 1;

    // descriptor set layout #0
    layout0.indices.push_back(0);
    layout0.descriptorTypes.push_back(vk::DescriptorType::eUniformBuffer);
    layout0.descriptorCounts.push_back(1);
    layout0.bindingStages.push_back(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    layout0.bindingFlags.emplace_back();
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(layout0));
    descriptorSetLayoutData_.push_back(layout0);

    DescriptorSetLayoutData layout1;
    layout1.descriptorSetCount = 1;

    // descriptor set layout #1
    layout1.indices.push_back(0);
    layout1.descriptorTypes.push_back(vk::DescriptorType::eUniformBufferDynamic);
    layout1.descriptorCounts.push_back(1);
    layout1.bindingStages.emplace_back(vk::ShaderStageFlagBits::eVertex);
    layout1.bindingFlags.emplace_back();
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(layout1));
    descriptorSetLayoutData_.push_back(layout1);

    DescriptorSetLayoutData layout2;
    layout2.descriptorSetCount = 1;

    // descriptor set layout #2
    layout2.indices.push_back(0);
    layout2.descriptorTypes.push_back(vk::DescriptorType::eCombinedImageSampler);
    layout2.descriptorCounts.push_back(Device::limits.maxDescriptorSetSamplers);
    layout2.bindingStages.emplace_back(vk::ShaderStageFlagBits::eFragment);
    layout2.bindingFlags.push_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
    layout2.layoutCreateFlags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(layout2));
    descriptorSetLayoutData_.push_back(layout2);

    descriptorPoolCreateFlags_ = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
}

void Viewport::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.format = vk::Format::eB8G8R8A8Srgb;
    colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentDescription depthAttachmentDesc;
    depthAttachmentDesc.format = vk::Format::eD32Sfloat;
    depthAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    depthAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eClear;
    depthAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachmentDesc.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<vk::AttachmentDescription, 2> attachments = { colorAttachmentDesc, depthAttachmentDesc };
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    renderPass_ = Device::GetHandle().device.createRenderPass(renderPassInfo);
}

void Viewport::CreateFrameBuffer()
{
    for (int i = 0; i < frames_.size(); ++i) {

        std::vector<vk::ImageView> attachments = {
            frames_[i].viewportImage.GetHandle().imageView,
            frames_[i].depthImage.GetHandle().imageView,
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = Swapchain::Get().swapchainImageExtent.width;
        framebufferInfo.height = Swapchain::Get().swapchainImageExtent.height;
        framebufferInfo.layers = 1;

        frames_[i].framebuffer = Device::GetHandle().device.createFramebuffer(framebufferInfo);
        Log(debug, fmt::terminal_color::bright_green, "created viewport framebuffer for frame {}", i);
    }
}

void Viewport::RecreateViewportImages()
{
    for (auto& frame : frames_) {
        Device::GetHandle().device.destroyFramebuffer(frame.framebuffer);

        frame.viewportImage.DestroyImage();
        frame.viewportImage.DestroyImageView();
        frame.viewportImage.memory.Free();

        frame.depthImage.DestroyImage();
        frame.depthImage.DestroyImageView();
        frame.depthImage.memory.Free();
    }

    for (auto& frame : frames_) {
        frame.viewportImage.CreateImage(vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eHostVisible, { Swapchain::Get().swapchainImageExtent.width, Swapchain::Get().swapchainImageExtent.height, 1 });
        frame.viewportImage.CreateImageView(vk::Format::eB8G8R8A8Srgb, vk::ImageAspectFlagBits::eColor);
        frame.viewportImage.SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);

        frame.depthImage.CreateImage(vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, { Swapchain::Get().swapchainImageExtent.width, Swapchain::Get().swapchainImageExtent.height, 1 });
        frame.depthImage.CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);
    }

    CreateFrameBuffer();
}

void Viewport::RecordDrawCommand(size_t frameIndex, const std::vector<Mesh>& meshes, uint32_t dynamicOffsetSize)
{
    auto& frame = frames_[frameIndex];
    auto& commandBuffer = frame.command.commandBuffers_.back();

    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = frame.framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = Swapchain::Get().swapchainImageExtent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue clearValue;
    clearValue.color = { std::array<float, 4>{ 1.0f, 1.0f, 1.0f, 1.0f } };
    vk::ClearValue depthClear;
    depthClear.depthStencil.depth = 1.0f;
    renderPassInfo.clearValueCount = 2;
    vk::ClearValue clearValues[] = { clearValue, depthClear };
    renderPassInfo.pClearValues = &clearValues[0];

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(Swapchain::Get().swapchainImageExtent.height);
    viewport.width = static_cast<float>(Swapchain::Get().swapchainImageExtent.width);
    viewport.height = -1.0f * static_cast<float>(Swapchain::Get().swapchainImageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = Swapchain::Get().swapchainImageExtent;

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.GetHandle().pipelineLayout, 0, 1, &frames_[frameIndex].descriptor.descriptorSets_[0], 0, nullptr);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.GetHandle().pipelineLayout, 2, 1, &frames_[frameIndex].descriptor.descriptorSets_[2], 0, nullptr);

    for (int i = 0; i < meshes.size(); i++) {

        vk::Buffer vertexBuffers[] = { meshes[i].vertexBuffer->GetHandle().buffer };
        vk::DeviceSize offsets[] = { 0 };

        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(meshes[i].indexBuffer->GetHandle().buffer, 0, vk::IndexType::eUint32);

        uint32_t dynamicOffset = i * dynamicOffsetSize;
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_.GetHandle().pipelineLayout, 1, 1, &frames_[frameIndex].descriptor.descriptorSets_[1], 1, &dynamicOffset);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.GetHandle().pipeline);

        commandBuffer.drawIndexed(static_cast<uint32_t>(meshes[i].GetIndexCount()), 1, 0, 0, 0);
    }

    commandBuffer.endRenderPass();
    commandBuffer.end();
}

Viewport::~Viewport()
{
    Device::GetHandle().device.destroyRenderPass(renderPass_);
    for (auto& layout : descriptorSetLayouts_)
        Device::GetHandle().device.destroyDescriptorSetLayout(layout);
    for (auto& frame : frames_)
        Device::GetHandle().device.destroyFramebuffer(frame.framebuffer);
}
