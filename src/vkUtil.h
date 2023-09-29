#ifndef _VKUTIL_H_
#define _VKUTIL_H_

#include "baseAppInfo.h"
#include "common.h"

class VkUtil
{
public:
    static VkUtil& Get()
    {
        static VkUtil vkUtil;
        return vkUtil;
    }

    void CreateInstance();
    std::vector<const char*> GetRequiredExtensions();

    void CreateSurface(GLFWwindow* window);

    void PickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
    void FindQueueFamilies(VkPhysicalDevice physicalDevice);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

    void CreateLogicalDevice();

    void QuerySwapChainSupport(VkPhysicalDevice physicalDevice);

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
    void CreateSwapChain(GLFWwindow* window);
    void CreateImageViews();
    void CreateRenderPass();
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void RecordCommandBuffer(uint32_t imageIndex);
    void CreateSyncObjects();

    VkInstance instance;                          // To interface with Vulkan library
    VkPhysicalDevice physicalDevice;              // Graphics Hardware
    VkDevice device;                              // Logical Device to interface with GPU
    VkQueue graphicsQueue;                        // Queue Handle to handle queue created with logical device
    VkSurfaceKHR surface;                         // To interface with window system
    VkQueue presentQueue;                         // Queue handle to interface with present queue
    VkSwapchainKHR swapChain;                     // Swap chain for image presentation, need to be requested and enabled as an extension
    std::vector<VkImage> swapChainImages;         // To retreive swap chain images
    VkFormat swapChainImageFormat;                // To retreive swap chain images
    VkExtent2D swapChainExtent;                   // Set resolution of swap chain
    std::vector<VkImageView> swapChainImageViews; // To use swap chain images
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

private:
    VkUtil() {}
    BaseAppInfo& s_baseAppInfo = BaseAppInfo::Get();
};

struct QueueFamilyIndices {

    static QueueFamilyIndices& Get()
    {
        static QueueFamilyIndices indices;
        return indices;
    }

    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

private:
    QueueFamilyIndices() {}
};

struct SwapChainSupportDetails {

    static SwapChainSupportDetails& Get()
    {
        static SwapChainSupportDetails details;
        return details;
    }

    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

private:
    SwapChainSupportDetails() {}
};

#endif