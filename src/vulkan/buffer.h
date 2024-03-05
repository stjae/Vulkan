#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include "device.h"
#include "memory.h"

struct BufferInput
{
    size_t size;
    vk::BufferUsageFlags usage;
    vk::MemoryPropertyFlags properties;
};
struct BufferBundle
{
    vk::Buffer buffer;
    vk::DescriptorBufferInfo bufferInfo;
    vk::DeviceMemory bufferMemory;
    vk::BufferUsageFlags bufferUsage;
};

class Buffer : public Memory
{
    BufferBundle bufferBundle_;
    size_t bufferSize_;

public:
    Buffer(const BufferInput& bufferInput);
    void MapMemory(vk::DeviceSize range);
    size_t GetSize() const { return bufferSize_; }
    void Destroy();
    ~Buffer();

    const BufferBundle& GetBundle() { return bufferBundle_; }
    static size_t GetBufferRange(size_t size, size_t minOffset);

    template <typename T>
    void CopyResourceToBuffer(T resource, const BufferInput& bufferInput)
    {
        auto* bufferMemoryAddress = static_cast<char*>(Device::GetBundle().device.mapMemory(GetMemory(), 0, bufferInput.size));

        SetAddress(bufferMemoryAddress);
        memcpy(bufferMemoryAddress, resource, bufferInput.size);
        Device::GetBundle().device.unmapMemory(GetMemory());

        // TODO: update bufferInfo
    }

    template <typename T>
    void UpdateBuffer(T resource, size_t size)
    {
        void* bufferMemoryAddress = GetMemoryAddress();
        memcpy(bufferMemoryAddress, resource, size);
    }
};

template <typename T>
struct DynamicUniformBuffer
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
    ~DynamicUniformBuffer() { AlignedFree(data); }
};

#endif