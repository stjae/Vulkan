#include "swapchain.h"
#include "instance.h"
#include "device.h"
#include "command.h"
#include "image.h"

namespace vkn {
Swapchain::Swapchain()
{
    Command::CreateCommandPool(m_commandPool);
    Command::AllocateCommandBuffer(m_commandPool, m_commandBuffer);

    CreateSwapchain();
    InitSwapchain();
    CreateRenderPass();
    CreateFrameBuffer();
}

void Swapchain::CreateSwapchain()
{
    QuerySwapchainSupport();
    ChooseSurfaceFormat();
    ChoosePresentMode();
    ChooseExtent();

    uint32_t imageCount = std::min(s_bundle.surfaceCapabilities.maxImageCount, s_bundle.surfaceCapabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo({}, Instance::GetSurface(), imageCount, s_bundle.surfaceFormat.format, s_bundle.surfaceFormat.colorSpace, s_bundle.swapchainImageExtent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t indices[] = { Device::Get().graphicsFamilyIndex.value(), Device::Get().presentFamilyIndex.value() };
    if (Device::Get().graphicsFamilyIndex.value() != Device::Get().presentFamilyIndex.value()) {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = indices;
    } else {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapchainCreateInfo.presentMode = s_bundle.presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = nullptr;

    s_bundle.swapchain = Device::Get().device.createSwapchainKHR(swapchainCreateInfo);
    Log(DEBUG, fmt::terminal_color::bright_green, "swapchain created");

    // Get swapchain image handle
    std::vector<vk::Image> swapchainImages = Device::Get().device.getSwapchainImagesKHR(s_bundle.swapchain);
    s_bundle.frameImageCount = swapchainImages.size();
    m_swapchainImages.resize(s_bundle.frameImageCount);

    for (size_t i = 0; i < swapchainImages.size(); ++i) {

        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        imageViewCreateInfo.components = vk::ComponentSwizzle::eIdentity;

        vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        imageViewCreateInfo.subresourceRange = range;
        imageViewCreateInfo.format = s_bundle.surfaceFormat.format;

        m_swapchainImages[i].image = swapchainImages[i];
        m_swapchainImages[i].imageView = Device::Get().device.createImageView(imageViewCreateInfo);
    }
}

void Swapchain::InitSwapchain()
{
    vkn::Command::Begin(m_commandBuffer);
    for (auto& swapchainImage : m_swapchainImages) {
        vkn::Command::SetImageMemoryBarrier(m_commandBuffer, swapchainImage.image, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, {}, {}, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe);
    }
    m_commandBuffer.end();
    vkn::Command::SubmitAndWait(m_commandBuffer);
}

void Swapchain::CreateRenderPass()
{
    vk::AttachmentDescription swapchainAttachment;
    swapchainAttachment.format = vk::Format::eB8G8R8A8Srgb;
    swapchainAttachment.samples = vk::SampleCountFlagBits::e1;
    swapchainAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    swapchainAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    swapchainAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    swapchainAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    swapchainAttachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    swapchainAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &swapchainAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    s_bundle.renderPass = Device::Get().device.createRenderPass(renderPassInfo);
}

void Swapchain::CreateFrameBuffer()
{
    for (int i = 0; i < m_swapchainImages.size(); ++i) {

        std::vector<vk::ImageView> attachments = {
            m_swapchainImages[i].imageView,
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = s_bundle.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = s_bundle.swapchainImageExtent.width;
        framebufferInfo.height = s_bundle.swapchainImageExtent.height;
        framebufferInfo.layers = 1;

        m_swapchainImages[i].framebuffer = Device::Get().device.createFramebuffer(framebufferInfo);
        Log(DEBUG, fmt::terminal_color::bright_green, "created framebuffer {}", i);
    }
}

void Swapchain::Draw(uint32_t imageIndex, ImDrawData* imDrawData, const vk::CommandBuffer& commandBuffer)
{
    auto& swapchainImage = m_swapchainImages[imageIndex];

    Command::SetImageMemoryBarrier(commandBuffer,
                                   swapchainImage.image,
                                   vk::ImageLayout::ePresentSrcKHR,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   {}, vk::AccessFlagBits::eColorAttachmentWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = s_bundle.renderPass;
    renderPassInfo.framebuffer = swapchainImage.framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = s_bundle.swapchainImageExtent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue clearValue;
    clearValue.color = { std::array<float, 4>{ 0.1f, 0.1f, 0.1f, 1.0f } };
    vk::ClearValue depthClear;
    depthClear.depthStencil.depth = 1.0f;
    renderPassInfo.clearValueCount = 2;
    vk::ClearValue clearValues[] = { clearValue, depthClear };
    renderPassInfo.pClearValues = &clearValues[0];

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(s_bundle.swapchainImageExtent.height);
    viewport.width = static_cast<float>(s_bundle.swapchainImageExtent.width);
    viewport.height = -1.0f * static_cast<float>(s_bundle.swapchainImageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = s_bundle.swapchainImageExtent;
    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    ImGui_ImplVulkan_RenderDrawData(imDrawData, commandBuffer);

    commandBuffer.endRenderPass();

    Command::SetImageMemoryBarrier(commandBuffer,
                                   swapchainImage.image,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::ImageLayout::ePresentSrcKHR,
                                   vk::AccessFlagBits::eColorAttachmentWrite, {},
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                   vk::PipelineStageFlagBits::eBottomOfPipe);
}

void Swapchain::Destroy()
{
    for (auto& swapchainImage : m_swapchainImages) {
        Device::Get().device.destroyImageView(swapchainImage.imageView);
        Device::Get().device.destroyFramebuffer(swapchainImage.framebuffer);
    }
    Device::Get().device.destroySwapchainKHR(s_bundle.swapchain);
}

Swapchain::~Swapchain()
{
    Destroy();
    Device::Get().device.destroyCommandPool(m_commandPool);
    Device::Get().device.destroyRenderPass(s_bundle.renderPass);
}

void Swapchain::QuerySwapchainSupport()
{
    s_bundle.surfaceCapabilities = Device::Get().physicalDevice.getSurfaceCapabilitiesKHR(Instance::GetSurface());
    m_supportedFormats = Device::Get().physicalDevice.getSurfaceFormatsKHR(Instance::GetSurface());
    m_supportedPresentModes = Device::Get().physicalDevice.getSurfacePresentModesKHR(Instance::GetSurface());

    Log(DEBUG, fmt::terminal_color::black, "printing queries for surface supports..");

    Log(DEBUG, fmt::terminal_color::white, "current surface m_extent width: {}", s_bundle.surfaceCapabilities.currentExtent.width);
    Log(DEBUG, fmt::terminal_color::white, "current surface m_extent height: {}", s_bundle.surfaceCapabilities.currentExtent.height);

    for (auto& mode : m_supportedPresentModes) {
        Log(DEBUG, fmt::terminal_color::white, "supported present mode: {}", vk::to_string(mode));
    }
}

void Swapchain::ChooseSurfaceFormat()
{
    Log(DEBUG, fmt::terminal_color::black, "setting swapchain details..");

    for (auto& format : m_supportedFormats) {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            Log(DEBUG, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(format.format));
            Log(DEBUG, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(format.colorSpace));

            s_bundle.surfaceFormat = format;
            return;
        }
    }

    Log(DEBUG, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(m_supportedFormats[0].format));
    Log(DEBUG, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(m_supportedFormats[0].colorSpace));

    s_bundle.surfaceFormat = m_supportedFormats[0].format;
}

void Swapchain::ChoosePresentMode()
{
    vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;

    Log(DEBUG, fmt::terminal_color::bright_cyan, "set swapchain present mode: {}", vk::to_string(mode));

    s_bundle.presentMode = mode;
}

void Swapchain::ChooseExtent()
{
    // m_extent is set
    if (s_bundle.surfaceCapabilities.currentExtent.width != UINT32_MAX) {

        Log(DEBUG, fmt::terminal_color::white, "no change in extent range");

        s_bundle.swapchainImageExtent = s_bundle.surfaceCapabilities.currentExtent;
        return;

    } else {
        // m_extent is not set
        int width, height;
        glfwGetFramebufferSize(Window::GetWindow(), &width, &height);

        vk::Extent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        if (extent.width > s_bundle.surfaceCapabilities.maxImageExtent.width) {

            extent.width = s_bundle.surfaceCapabilities.maxImageExtent.width;
            Log(DEBUG, fmt::terminal_color::yellow, "m_extent width is clamped");
        }
        if (extent.height > s_bundle.surfaceCapabilities.maxImageExtent.height) {

            extent.height = s_bundle.surfaceCapabilities.maxImageExtent.height;
            Log(DEBUG, fmt::terminal_color::yellow, "m_extent height is clamped");
        }

        s_bundle.swapchainImageExtent = extent;
        return;
    }
}

}; // namespace vkn