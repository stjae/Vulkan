#include "engine.h"

void Engine::InitSwapchainImages()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    for (auto& frame : swapchain_.frames) {
        Command::Begin(frame.commandBuffer);
        Command::SetImageMemoryBarrier(frame.commandBuffer, frame.swapchainImage, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, {}, {}, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe);
        frame.commandBuffer.end();
        Command::Submit(&frame.commandBuffer, 1);

        Device::GetBundle().device.waitIdle();
    }
}

void Engine::SetUp()
{
    scene_ = std::make_unique<Scene>();
    imgui_.Setup(swapchain_.GetRenderPass(), viewport_);
}

void Engine::Update()
{
    scene_->Update();

    vk::WriteDescriptorSet cameraWrite(viewport_.GetPipelineState().meshRender.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &scene_->camera.GetBufferInfo(), nullptr, nullptr);
    vk::WriteDescriptorSet cameraWriteNormal(viewport_.GetPipelineState().normalRender.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &scene_->camera.GetBufferInfo(), nullptr, nullptr);
    vk::WriteDescriptorSet lightWrite(viewport_.GetPipelineState().meshRender.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &scene_->light.GetBufferInfo(), nullptr, nullptr);
    std::array<vk::WriteDescriptorSet, 3> globalWrite{ cameraWrite, lightWrite, cameraWriteNormal };
    Device::GetBundle().device.updateDescriptorSets(globalWrite, nullptr);

    if (scene_->meshes.empty())
        return;

    vk::WriteDescriptorSet modelMatrixWrite(
        viewport_.GetPipelineState().meshRender.descriptorSets[1], 0, 0, 1,
        vk::DescriptorType::eUniformBufferDynamic, nullptr,
        &scene_->meshDynamicUniformBuffer_->GetBundle().bufferInfo, nullptr, nullptr);
    vk::WriteDescriptorSet modelMatrixWriteNormal(
        viewport_.GetPipelineState().normalRender.descriptorSets[1], 0, 0, 1,
        vk::DescriptorType::eUniformBufferDynamic, nullptr,
        &scene_->meshDynamicUniformBuffer_->GetBundle().bufferInfo, nullptr, nullptr);

    std::array<vk::WriteDescriptorSet, 2> dynamicWrite{ modelMatrixWrite, modelMatrixWriteNormal };
    Device::GetBundle().device.updateDescriptorSets(dynamicWrite, nullptr);

    for (auto& mesh : scene_->meshes) {

        vk::DescriptorBufferInfo storageBufferInInfo(mesh.vertexBuffer->GetBundle().buffer, 0, mesh.vertexBuffer->GetSize());
        vk::DescriptorBufferInfo storageBufferOutInfo(mesh.vertexStorageBuffer->GetBundle().buffer, 0, mesh.vertexStorageBuffer->GetSize());

        std::array<vk::WriteDescriptorSet, 2> computeDescriptorWrites;
        computeDescriptorWrites[0] = { viewport_.GetPipelineState().compute.descriptorSets[0], 0, 0, 1,
                                       vk::DescriptorType::eStorageBuffer, nullptr,
                                       &storageBufferInInfo, nullptr, nullptr };
        computeDescriptorWrites[1] = { viewport_.GetPipelineState().compute.descriptorSets[0], 1, 0, 1,
                                       vk::DescriptorType::eStorageBuffer, nullptr,
                                       &storageBufferOutInfo, nullptr, nullptr };

        Device::GetBundle().device.updateDescriptorSets(computeDescriptorWrites.size(), computeDescriptorWrites.data(), 0, nullptr);
    }

    vk::WriteDescriptorSet descriptorWrites;
    descriptorWrites.dstSet = viewport_.GetPipelineState().meshRender.descriptorSets[2];
    descriptorWrites.dstBinding = 0;
    descriptorWrites.dstArrayElement = 0;
    descriptorWrites.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrites.descriptorCount = scene_->textures.size();
    std::vector<vk::DescriptorImageInfo> infos;
    for (auto& texture : scene_->textures) {
        infos.push_back(texture->image->GetBundle().imageInfo);
    }
    descriptorWrites.pImageInfo = infos.data();
    Device::GetBundle().device.updateDescriptorSets(descriptorWrites, nullptr);
}

void Engine::Render()
{
    Device::GetBundle().device.waitForFences(1, &swapchain_.frames[frameIndex_].inFlight, VK_TRUE, UINT64_MAX);

    auto waitFrameImage = Device::GetBundle().device.acquireNextImageKHR(Swapchain::GetBundle().swapchain, UINT64_MAX, swapchain_.frames[frameIndex_].imageAvailable, nullptr);

    if (IsSwapchainOutOfDate(waitFrameImage))
        return;

    Device::GetBundle().device.resetFences(1, &swapchain_.frames[frameIndex_].inFlight);

    swapchain_.Draw(frameIndex_, imDrawData_);
    viewport_.Draw(frameIndex_, *scene_);
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && viewport_.isMouseHovered)
        scene_->selectedMeshIndex = viewport_.PickColor(frameIndex_);
    swapchain_.Submit(frameIndex_);
    auto presentInfo = swapchain_.Present(frameIndex_, waitFrameImage);

    // if (Device::GetBundle().presentQueue.presentKHR(presentInfo) == vk::Result::eErrorOutOfDateKHR) {
    //     RecreateSwapchain();
    //     InitSwapchainImages();
    //
    //     return;
    // }

    frameIndex_ = (frameIndex_ + 1) % Swapchain::GetBundle().frameImageCount;
}

bool Engine::IsSwapchainOutOfDate(const vk::ResultValue<unsigned int>& waitFrameImage)
{
    if (waitFrameImage.result == vk::Result::eErrorOutOfDateKHR || waitFrameImage.result == vk::Result::eSuboptimalKHR) {
        RecreateSwapchain();
        InitSwapchainImages();
        viewport_.outDated = true;
        viewport_.DestroyViewportImages();
        viewport_.CreateViewportImages();
        imgui_.RecreateViewportDescriptorSets(viewport_);
        return true;
    } else
        return false;
}

void Engine::RecreateSwapchain()
{
    int width = 0;
    int height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(Window::GetWindow(), &width, &height);
        glfwWaitEvents();
    }

    Device::GetBundle().device.waitIdle();
    for (auto& frame : swapchain_.frames) {
        Device::GetBundle().device.freeCommandBuffers(frame.commandPool, frame.commandBuffer);
        Device::GetBundle().device.destroyCommandPool(frame.commandPool);
    }
    swapchain_.Destroy();

    swapchain_.CreateSwapchain();
    swapchain_.CreateFrameBuffer();

    swapchain_.PrepareFrames();
}

void Engine::DrawUI()
{
    imgui_.Draw(*scene_, viewport_, frameIndex_);
    imDrawData_ = ImGui::GetDrawData();
}

Engine::~Engine()
{
    Device::GetBundle().device.waitIdle();
}
