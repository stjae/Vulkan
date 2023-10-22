#ifndef _SWAPCHAIN_H_
#define _SWAPCHAIN_H_

#include "../common.h"
#include "device.h"
#include "instance.h"
#include "memory.h"
#include "pipeline.h"
#include "sync.h"

class Swapchain
{
public:
    ~Swapchain();
    void CreateSwapchain(GLFWwindow* window);
    void QuerySwapchainSupportDetails();
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent(GLFWwindow* window);
    void CreateFrameBuffer();
    void PrepareFrames();
    void DestroySwapchain();
};

struct SwapchainSupportDetails {

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct UBO {

    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewProj;
};

struct SwapchainFrame {

    vk::Image image;
    vk::ImageView imageView;
    vk::Framebuffer framebuffer;

    vk::CommandBuffer commandBuffer;

    vk::Fence inFlight;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;

    UBO cameraData;
    Buffer cameraDataBuffer;
    void* cameraDataMemoryLocation;

    vk::DescriptorBufferInfo uniformBufferDescriptorInfo;
    vk::DescriptorSet descriptorSet;

    void CreateResource()
    {
        BufferInput input;
        input.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        input.size = sizeof(UBO);
        input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
        cameraDataBuffer = CreateBuffer(input);

        cameraDataMemoryLocation = device.mapMemory(cameraDataBuffer.memory, 0, sizeof(UBO));

        uniformBufferDescriptorInfo.buffer = cameraDataBuffer.buffer;
        uniformBufferDescriptorInfo.offset = 0;
        uniformBufferDescriptorInfo.range = sizeof(UBO);
    }

    void WriteDescriptorSet()
    {
        vk::WriteDescriptorSet writeInfo(descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, uniformBufferDescriptorInfo);

        device.updateDescriptorSets(writeInfo, nullptr);
    }
};

struct SwapchainDetails {

    vk::SwapchainKHR swapchain;
    std::vector<SwapchainFrame> frames;
    vk::Format imageFormat;
    vk::Extent2D extent;
};

inline SwapchainSupportDetails swapchainSupportDetails;
inline SwapchainDetails swapchainDetails;

#endif