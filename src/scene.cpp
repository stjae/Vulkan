#include "scene.h"
extern size_t dynamicBufferAlignment;

Scene::Scene()
{
    meshes.emplace_back(std::make_shared<Mesh>());
    meshes.back()->CreateCube(nullptr);
    meshNames.push_back(meshes.back()->name.c_str());
    // meshes.emplace_back(std::make_shared<Mesh>());
    // meshes.back()->LoadModel("models/viking_room.obj", "textures/viking_room.png");
    // meshNames.push_back(meshes.back()->name.c_str());
}

void Scene::PrepareUniformBuffers()
{
    size_t minUboAlignment = Device::GetPhysicalDevice()
                                 .getProperties()
                                 .limits.minUniformBufferOffsetAlignment;
    dynamicBufferAlignment = sizeof(glm::mat4);

    if (minUboAlignment > 0) {
        // TODO
        dynamicBufferAlignment =
            (dynamicBufferAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    size_t bufferSize = meshes.size() * dynamicBufferAlignment;
    uboDataDynamic.model = (glm::mat4*)aligned_alloc(dynamicBufferAlignment, bufferSize);
    if (uboDataDynamic.model == nullptr) {
        spdlog::error("failed to allocate memory for dynamic buffer");
    }
}

void Scene::Prepare()
{
    PrepareUniformBuffers();
    for (int i = 0; i < meshes.size(); i++) {
        glm::mat4* modelMat =
            (glm::mat4*)((uint64_t)uboDataDynamic.model + (i * dynamicBufferAlignment));
        *modelMat = glm::mat4(1.0f);
        std::cout << glm::to_string(*uboDataDynamic.model) << std::endl;
    }

    camera = std::make_unique<Camera>();
    CreateResource();

    Command command;
    command.CreateCommandPool("copying buffers");

    for (auto& mesh : meshes) {
        command.RecordCopyCommands(mesh->vertexStagingBuffer->GetBuffer(),
                                   mesh->vertexBuffer->GetBuffer(),
                                   sizeof(mesh->vertices[0]) * mesh->vertices.size());
        command.RecordCopyCommands(mesh->indexStagingBuffer->GetBuffer(),
                                   mesh->indexBuffer->GetBuffer(),
                                   sizeof(mesh->indices[0]) * mesh->indices.size());
        command.TransitImageLayout(mesh->textureImage->GetImage(),
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal);
    }

    command.Submit();

    for (auto& mesh : meshes)
        command.RecordCopyCommands(mesh->textureStagingBuffer->GetBuffer(),
                                   mesh->textureImage->GetImage(), mesh->textureWidth,
                                   mesh->textureHeight, mesh->textureSize);

    command.Submit();

    for (auto& mesh : meshes) {
        command.TransitImageLayout(mesh->textureImage->GetImage(),
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->textureImage->SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->DestroyStagingBuffer();
    }

    command.Submit();
}

void Scene::CreateResource()
{
    for (auto& mesh : meshes) {
        mesh->CreateIndexBuffer();
        mesh->CreateVertexBuffer();

        mesh->CreateTexture();
        mesh->textureImage->CreateSampler();
    }

    BufferInput input = { meshes.size() * dynamicBufferAlignment,
                          vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible };
    matrixUniformBufferDynamic = std::make_unique<Buffer>(input);
    matrixUniformBufferDynamic->Map(dynamicBufferAlignment);
}

void Scene::Update(uint32_t index)
{
    if (camera->isControllable) {
        camera->Update();
    }

    camera->matrix_.view = glm::lookAt(camera->pos, camera->at, camera->up);
    camera->matrix_.proj =
        glm::perspective(glm::radians(45.0f),
                         static_cast<float>(Swapchain::GetDetail().extent.width) /
                             static_cast<float>(Swapchain::GetDetail().extent.height),
                         0.1f, 100.0f);
    camera->UpdateBuffer();

    vk::WriteDescriptorSet cameraMatrixWrite(
        Swapchain::GetDetail().frames[index].descriptorSets[0], 0, 0, 1,
        vk::DescriptorType::eUniformBuffer, nullptr, &camera->GetBufferInfo(), nullptr,
        nullptr);
    Device::GetDevice().updateDescriptorSets(cameraMatrixWrite, nullptr);

    UpdateBuffer();
    vk::MappedMemoryRange memoryRange;
    memoryRange.memory = matrixUniformBufferDynamic->GetBufferMemory();
    memoryRange.size = sizeof(uboDataDynamic);
    auto result = Device::GetDevice().flushMappedMemoryRanges(1, &memoryRange);

    // for (auto& mesh : meshes) {
    vk::WriteDescriptorSet modelMatrixWrite(
        Swapchain::GetDetail().frames[index].descriptorSets[0], 1, 0, 1,
        vk::DescriptorType::eUniformBufferDynamic, nullptr,
        &matrixUniformBufferDynamic->GetBufferInfo(), nullptr, nullptr);
    Device::GetDevice().updateDescriptorSets(modelMatrixWrite, nullptr);
    // }

    // vk::WriteDescriptorSet descriptorWrites;
    // descriptorWrites.dstSet = Swapchain::GetDetail().frames[index].descriptorSets[0];
    // descriptorWrites.dstBinding = 2;
    // descriptorWrites.dstArrayElement = 0;
    // descriptorWrites.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    // descriptorWrites.descriptorCount = meshes.size();
    // std::vector<vk::DescriptorImageInfo> infos;
    // for (auto& mesh : meshes) {
    //     infos.push_back(mesh->textureImage->GetInfo());
    // }
    // descriptorWrites.pImageInfo = infos.data();
    // if (meshes.size() > 0) {
    //     Device::GetDevice().updateDescriptorSets(descriptorWrites, nullptr);
    // }
}

void Scene::UpdateBuffer()
{
    matrixUniformBufferDynamic->UpdateBuffer(&uboDataDynamic,
                                             matrixUniformBufferDynamic->GetBufferSize());
}

Scene::~Scene() { free(uboDataDynamic.model); }