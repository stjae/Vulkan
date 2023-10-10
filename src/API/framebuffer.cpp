#include "framebuffer.h"

void Framebuffer::CreateFramebuffer()
{
    auto& frames = Swapchain::swapchainDetails.frames;

    for (int i = 0; i < frames.size(); ++i) {

        std::vector<vk::ImageView> attachments = { frames[i].imageView };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.setRenderPass(GraphicsPipeline::output.renderPass);
        framebufferInfo.setAttachmentCount(attachments.size());
        framebufferInfo.setPAttachments(attachments.data());
        framebufferInfo.setWidth(GraphicsPipeline::input.swapchainExtent.width);
        framebufferInfo.setHeight(GraphicsPipeline::input.swapchainExtent.height);
        framebufferInfo.setLayers(1);

        frames[i].framebuffer = Device::device.createFramebuffer(framebufferInfo);
        spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::bright_green), "created framebuffer for frame {}", i));
    }
}
