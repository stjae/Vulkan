#include "baseApp.h"

void HelloTriangleApplication::run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    CleanUp();
}

void HelloTriangleApplication::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::InitVulkan()
{
    s_vkUtil.CreateInstance();

    s_vkDebug.SetupDebugMessenger();

    s_vkUtil.CreateSurface(m_window);

    s_vkUtil.PickPhysicalDevice();

    s_vkUtil.CreateLogicalDevice();
    s_vkUtil.CreateSwapChain(m_window);
    s_vkUtil.CreateImageViews();
    s_vkUtil.CreateRenderPass();
    s_vkUtil.CreateGraphicsPipeline();

    s_vkUtil.CreateFramebuffers();

    s_vkUtil.CreateCommandPool();
    s_vkUtil.CreateCommandBuffer();

    s_vkUtil.CreateSyncObjects();
}

void HelloTriangleApplication::MainLoop()
{
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(s_vkUtil.device);
}

void HelloTriangleApplication::DrawFrame()
{
    vkWaitForFences(s_vkUtil.device, 1, &s_vkUtil.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(s_vkUtil.device, 1, &s_vkUtil.inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(s_vkUtil.device, s_vkUtil.swapChain, UINT64_MAX, s_vkUtil.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(s_vkUtil.commandBuffer, 0);
    s_vkUtil.RecordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { s_vkUtil.imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &s_vkUtil.commandBuffer;

    VkSemaphore signalSemaphores[] = { s_vkUtil.renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(s_vkUtil.graphicsQueue, 1, &submitInfo, s_vkUtil.inFlightFence) != VK_SUCCESS) {
        spdlog::error("failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { s_vkUtil.swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(s_vkUtil.presentQueue, &presentInfo);
}

void HelloTriangleApplication::CleanUp()
{
    vkDestroySemaphore(s_vkUtil.device, s_vkUtil.imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(s_vkUtil.device, s_vkUtil.renderFinishedSemaphore, nullptr);
    vkDestroyFence(s_vkUtil.device, s_vkUtil.inFlightFence, nullptr);

    vkDestroyCommandPool(s_vkUtil.device, s_vkUtil.commandPool, nullptr);

    for (auto framebuffer : s_vkUtil.swapChainFramebuffers) {
        vkDestroyFramebuffer(s_vkUtil.device, framebuffer, nullptr);
    }

    vkDestroyPipeline(s_vkUtil.device, s_vkUtil.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(s_vkUtil.device, s_vkUtil.pipelineLayout, nullptr);
    vkDestroyRenderPass(s_vkUtil.device, s_vkUtil.renderPass, nullptr);

    for (auto imageView : s_vkUtil.swapChainImageViews) {
        vkDestroyImageView(s_vkUtil.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(s_vkUtil.device, s_vkUtil.swapChain, nullptr);
    vkDestroyDevice(s_vkUtil.device, nullptr);

    if (s_baseAppInfo.enableValidationLayers) {
        s_vkDebug.DestroyDebugUtilsMessengerEXT(nullptr);
    }

    vkDestroySurfaceKHR(s_vkUtil.instance, s_vkUtil.surface, nullptr);
    vkDestroyInstance(s_vkUtil.instance, nullptr);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}
