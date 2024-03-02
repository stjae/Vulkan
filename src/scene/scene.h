#ifndef SCENE_H
#define SCENE_H

#include "../common.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "light.h"
#include "../vulkan/swapchain.h"
#include "../vulkan/command.h"
#include "../../imgui/imgui_impl_vulkan.h"

template <typename T>
struct BufferArray
{
    T* data;
    std::unique_ptr<Buffer> buffer;
    size_t bufferRange = 0;
    size_t bufferSize = 0;

    void CreateBuffer(size_t vectorSize, glm::vec3 pos, uint32_t id, vk::BufferUsageFlags bufferUsage)
    {
        if (vectorSize < 1)
            return;

        // create buffer data
        size_t requiredSize = vectorSize * bufferRange;
        if (bufferSize < requiredSize) {
            void* newAlignedMemory = AlignedAlloc(bufferRange, requiredSize);
            if (!newAlignedMemory)
                spdlog::error("failed to allocate dynamic uniform buffer memory");

            if (buffer != nullptr) {
                memcpy(newAlignedMemory, data, bufferSize);
                AlignedFree(data);
            }

            data = (T*)newAlignedMemory;
        }

        size_t newIndex = vectorSize - 1;
        auto* newData = (T*)((uint64_t)data + (newIndex * bufferRange));
        *newData = T(pos, id);

        // create buffer
        if (bufferSize < requiredSize) {
            buffer.reset();
            BufferInput input = { vectorSize * bufferRange,
                                  bufferUsage,
                                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
            buffer = std::make_unique<Buffer>(input);
            buffer->MapMemory(bufferRange);

            bufferSize = requiredSize;
        }
    }
    void RearrangeBuffer(size_t index, size_t vectorSize) const
    {
        // move the meshUniformData forward as the mesh index refers to has been deleted
        if (index + 1 < vectorSize)
            memcpy((T*)((uint64_t)data + index * bufferRange), (T*)((uint64_t)data + (index + 1) * bufferRange), (vectorSize - index + 1) * bufferRange);
    }
};

struct Texture
{
    std::unique_ptr<Buffer> stagingBuffer;
    std::unique_ptr<Image> image;
    vk::DescriptorSet descriptorSet; // thumbnail for resource window

    int width{}, height{};
    size_t size{};
    int32_t index;

    ~Texture() { ImGui_ImplVulkan_RemoveTexture(descriptorSet); }
};

struct Resource
{
    std::string fileName;
    std::string fileFormat;
    std::string filePath;
    TypeEnum::Resource resourceType;
    // pointer to resource
    std::shared_ptr<void> resource;

    Resource(std::string& path)
    {
        this->filePath = path;
        this->fileName = path.substr(path.rfind('/') + 1, path.rfind('.') - path.rfind('/') - 1);
        this->fileFormat = path.substr(path.rfind('.') + 1, path.size());
    }
};

class Scene
{
    friend class Engine;
    friend class UI;
    friend class Viewport;

    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    BufferArray<MeshUniformData> meshDynamicUniformBuffer_;

    void CreateDummyTexture();
    void PrepareMesh(Mesh& mesh);

public:
    std::vector<Mesh> meshes;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<Resource> resources;
    Camera camera;
    Light light;
    int32_t selectedMeshIndex = -1;

    Scene();
    void AddMesh(TypeEnum::Mesh type);
    void AddMesh(TypeEnum::Mesh type, const std::string& filePath);
    void AddTexture(const std::string& filePath);
    const char* GetMeshName(size_t index) const { return meshes[index].name_.c_str(); }
    void DeleteMesh(size_t index);
    void Update();
    ~Scene();
};

#endif