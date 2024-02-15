#include "engine.h"

void Engine::InitSwapchainImages()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    for (auto& frame : swapchain_.frames) {
        frame.commandBuffer.begin(&beginInfo);

        vk::ImageMemoryBarrier barrier;

        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = Device::GetBundle().graphicsFamilyIndex.value();
        barrier.dstQueueFamilyIndex = Device::GetBundle().graphicsFamilyIndex.value();
        barrier.image = frame.swapchainImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        frame.commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, {}, 0, nullptr, 0, nullptr, 1, &barrier);

        frame.commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.commandBuffer;

        Device::GetBundle().graphicsQueue.submit(1, &submitInfo, {});

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

    vk::WriteDescriptorSet cameraWrite(viewport_.frames[frameIndex_].descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &scene_->camera.GetBufferInfo(), nullptr, nullptr);
    vk::WriteDescriptorSet lightWrite(viewport_.frames[frameIndex_].descriptorSets[0], 1, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &scene_->light.GetBufferInfo(), nullptr, nullptr);
    std::array<vk::WriteDescriptorSet, 2> globalWrite{ cameraWrite, lightWrite };
    Device::GetBundle().device.updateDescriptorSets(globalWrite, nullptr);

    if (scene_->meshes.empty())
        return;

    vk::WriteDescriptorSet modelMatrixWrite(
        viewport_.frames[frameIndex_].descriptorSets[1], 0, 0, 1,
        vk::DescriptorType::eUniformBufferDynamic, nullptr,
        &scene_->meshUniformBufferDynamic_->GetBundle().bufferInfo, nullptr, nullptr);
    Device::GetBundle().device.updateDescriptorSets(modelMatrixWrite, nullptr);

    vk::WriteDescriptorSet descriptorWrites;
    descriptorWrites.dstSet = viewport_.frames[frameIndex_].descriptorSets[2];
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
        scene_->meshSelected = viewport_.PickColor(frameIndex_);
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
