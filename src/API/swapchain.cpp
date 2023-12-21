#include "swapchain.h"

void Swapchain::CreateSwapchain()
{
    QuerySwapchainSupportDetail();

    auto& capabilities = supportDetail_.capabilities;

    auto surfaceFormat = ChooseSurfaceFormat();
    auto presentMode = ChoosePresentMode();
    auto extent = ChooseExtent();

    uint32_t imageCount = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR createInfo({}, Instance::GetHandle().surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t indices[] = { Queue::GetGraphicsQueueFamilyIndex(), Queue::GetPresentQueueFamilyIndex() };
    if (Queue::GetGraphicsQueueFamilyIndex() != Queue::GetPresentQueueFamilyIndex()) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr;

    handle_ = Device::GetHandle().device.createSwapchainKHR(createInfo);
    Log(debug, fmt::terminal_color::bright_green, "swapchain created");

    std::vector<vk::Image> images = Device::GetHandle().device.getSwapchainImagesKHR(GetHandle());
    detail_.frames.resize(images.size());
    detail_.imageFormat = surfaceFormat.format;
    detail_.extent = extent;

    for (size_t i = 0; i < images.size(); ++i) {

        vk::ImageViewCreateInfo createInfo;
        createInfo.image = images[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.components = vk::ComponentSwizzle::eIdentity;

        vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        createInfo.subresourceRange = range;
        createInfo.format = surfaceFormat.format;

        detail_.frames[i].swapchainVkImage = images[i];
        detail_.frames[i].swapchainVkImageView = Device::GetHandle().device.createImageView(createInfo);

        vk::Extent3D extent = { detail_.extent.width, detail_.extent.height, 1 };
        if (detail_.frames[i].depthImage.GetHandle().image != VK_NULL_HANDLE) {
            detail_.frames[i].depthImage.DestroyImage();
            detail_.frames[i].depthImage.memory.Free();
        }
        if (detail_.frames[i].depthImage.GetHandle().imageView != VK_NULL_HANDLE) {
            detail_.frames[i].depthImage.DestroyImageView();
        }
        detail_.frames[i].depthImage.CreateImage(vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, extent);
        detail_.frames[i].depthImage.CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);
        detail_.depthImageFormat = vk::Format::eD32Sfloat;
    }
}

void Swapchain::QuerySwapchainSupportDetail()
{
    supportDetail_.capabilities = Device::GetHandle().physicalDevice.getSurfaceCapabilitiesKHR(Instance::GetHandle().surface);
    supportDetail_.formats = Device::GetHandle().physicalDevice.getSurfaceFormatsKHR(Instance::GetHandle().surface);
    supportDetail_.presentModes = Device::GetHandle().physicalDevice.getSurfacePresentModesKHR(Instance::GetHandle().surface);

    Log(debug, fmt::terminal_color::black, "printing queries for surface supports..");

    Log(debug, fmt::terminal_color::white, "current surface extent width: {}", supportDetail_.capabilities.currentExtent.width);
    Log(debug, fmt::terminal_color::white, "current surface extent height: {}", supportDetail_.capabilities.currentExtent.height);

    for (auto& mode : supportDetail_.presentModes) {
        Log(debug, fmt::terminal_color::white, "supported present mode: {}", vk::to_string(mode));
    }
}

vk::SurfaceFormatKHR Swapchain::ChooseSurfaceFormat()
{
    Log(debug, fmt::terminal_color::black, "setting swapchain details..");

    for (auto& format : supportDetail_.formats) {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(format.format));
            Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(format.colorSpace));
            return format;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(supportDetail_.formats[0].format));
    Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(supportDetail_.formats[0].colorSpace));
    return supportDetail_.formats[0];
}

vk::PresentModeKHR Swapchain::ChoosePresentMode()
{
    vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;

    for (auto& presentMode : supportDetail_.presentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            mode = vk::PresentModeKHR::eMailbox;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set swapchain present mode: {}", vk::to_string(mode));
    return mode;
}

vk::Extent2D Swapchain::ChooseExtent()
{
    auto& capabilities = supportDetail_.capabilities;

    // extent is set
    if (capabilities.currentExtent.width != UINT32_MAX) {

        Log(debug, fmt::terminal_color::white, "no change in extent size");
        return capabilities.currentExtent;

    } else {
        // extent is not set
        int width, height;
        glfwGetFramebufferSize(*Window::GetWindow(), &width, &height);

        vk::Extent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        if (extent.width > capabilities.maxImageExtent.width) {

            extent.width = capabilities.maxImageExtent.width;
            Log(debug, fmt::terminal_color::yellow, "extent width is clamped");
        }
        if (extent.height > capabilities.maxImageExtent.height) {

            extent.height = capabilities.maxImageExtent.height;
            Log(debug, fmt::terminal_color::yellow, "extent height is clamped");
        }

        return extent;
    }
}

void Swapchain::CreateFrameBuffer(const vk::RenderPass& vkRenderPass)
{
    for (int i = 0; i < detail_.frames.size(); ++i) {

        std::vector<vk::ImageView> attachments = {
            detail_.frames[i].swapchainVkImageView,
            detail_.frames[i].depthImage.GetHandle().imageView
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = vkRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = detail_.extent.width;
        framebufferInfo.height = detail_.extent.height;
        framebufferInfo.layers = 1;

        detail_.frames[i].framebuffer = Device::GetHandle().device.createFramebuffer(framebufferInfo);
        Log(debug, fmt::terminal_color::bright_green, "created framebuffer for frame {}", i);
    }
}

void Swapchain::PrepareFrames()
{
    for (auto& frame : detail_.frames) {
        frame.inFlight = MakeFence();
        frame.imageAvailable = MakeSemaphore();
        frame.renderFinished = MakeSemaphore();
    }
}

void Swapchain::DestroySwapchain()
{
    for (auto& frame : detail_.frames) {
        Device::GetHandle().device.destroyImageView(frame.swapchainVkImageView);
        Device::GetHandle().device.destroyFramebuffer(frame.framebuffer);
        Device::GetHandle().device.destroyFence(frame.inFlight);
        Device::GetHandle().device.destroySemaphore(frame.imageAvailable);
        Device::GetHandle().device.destroySemaphore(frame.renderFinished);
        frame.depthImage.memory.Free();
        frame.depthImage.DestroyImage();
        frame.depthImage.DestroyImageView();
    }

    Device::GetHandle().device.destroySwapchainKHR(GetHandle());
}

Swapchain::~Swapchain()
{
    DestroySwapchain();
}