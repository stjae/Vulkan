#include "framebuffer.h"

void Framebuffer::CreateFramebuffer()
{
    auto& frames = swapchainDetails.frames;

    for (int i = 0; i < frames.size(); ++i) {

        std::vector<vk::ImageView> attachments = { frames[i].imageView };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.setRenderPass(renderPass);
        framebufferInfo.setAttachmentCount((uint32_t)attachments.size());
        framebufferInfo.setPAttachments(attachments.data());
        framebufferInfo.setWidth(swapchainDetails.extent.width);
        framebufferInfo.setHeight(swapchainDetails.extent.height);
        framebufferInfo.setLayers(1);

        frames[i].framebuffer = device.createFramebuffer(framebufferInfo);
        Log(debug, fmt::terminal_color::bright_green, "created framebuffer for frame {}", i);
    }
}
