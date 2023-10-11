#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "../common.h"

// device
struct QueueFamilyIndices {

    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    QueueFamilyIndices() {}
    QueueFamilyIndices(const QueueFamilyIndices&) = delete;
    QueueFamilyIndices& operator=(const QueueFamilyIndices&) = delete;
};

inline vk::PhysicalDevice physicalDevice;
inline std::vector<const char*> deviceExtensions;
inline QueueFamilyIndices queueFamilyIndices;
inline vk::Device device;
inline vk::Queue graphicsQueue;
inline vk::Queue presentQueue;

// instance
inline vk::Instance instance;
inline vk::DispatchLoaderDynamic dldi;
inline std::vector<const char*> instanceExtensions;
inline std::vector<const char*> instanceLayers;
inline VkSurfaceKHR surface;

// swapchain
struct SwapchainSupportDetails {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct SwapchainFrame {

    vk::Image image;
    vk::ImageView imageView;
    vk::Framebuffer framebuffer;
    vk::CommandBuffer commandBuffer;
};

struct SwapchainDetails {

    vk::SwapchainKHR swapchain;
    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Extent2D extent;
};

inline SwapchainSupportDetails swapchainSupportDetails;
inline SwapchainDetails swapchainDetails;

// pipeline
inline vk::Pipeline graphicsPipeline;
inline vk::PipelineLayout pipelineLayout;
inline vk::RenderPass renderPass;

#endif