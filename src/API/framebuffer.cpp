#include "framebuffer.h"

void Framebuffer::CreateFramebuffer()
{
    auto& frames = swapchainDetails.frames;

    for (int i = 0; i < frames.size(); ++i) {

        std::vector<vk::ImageView> attachments = { frames[i].imageView };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainDetails.extent.width;
        framebufferInfo.height = swapchainDetails.extent.height;
        framebufferInfo.layers = 1;

        frames[i].framebuffer = device.createFramebuffer(framebufferInfo);
        Log(debug, fmt::terminal_color::bright_green, "created framebuffer for frame {}", i);
    }
}
