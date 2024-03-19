#include "ui.h"
#include "font/IconsFontAwesome5.h"

void UI::Setup(const vk::RenderPass& renderPass, Viewport& viewport, Scene& scene)
{
    Command::CreateCommandPool(commandPool_);
    Command::AllocateCommandBuffer(commandPool_, commandBuffer_);

    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<DescriptorBinding> imGuiBindings;
    imGuiBindings.emplace_back(0, vk::DescriptorType::eCombinedImageSampler, 100);
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(imGuiBindings));
    Descriptor::SetDescriptorPoolSize(poolSizes, imGuiBindings, maxSets);

    Descriptor::CreateDescriptorPool(descriptorPool_, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();

    ImGui_ImplGlfw_InitForVulkan(Window::GetWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Instance::GetBundle().instance;
    init_info.PhysicalDevice = Device::GetBundle().physicalDevice;
    init_info.Device = Device::GetBundle().device;
    init_info.QueueFamily = Device::GetBundle().graphicsComputeFamilyIndex.value();
    init_info.Queue = Device::GetBundle().graphicsQueue;
    init_info.DescriptorPool = descriptorPool_;
    init_info.Subpass = 0;
    init_info.MinImageCount = Swapchain::capabilities.minImageCount;
    init_info.ImageCount = Swapchain::GetBundle().frameImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    io.Fonts->AddFontDefault();
    float baseFontSize = 13.0f;
    float iconFontSize = baseFontSize * 2.0f;

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    std::string path(PROJECT_DIR "ttf/");
    path.append(FONT_ICON_FILE_NAME_FAS);
    io.Fonts->AddFontFromFileTTF(path.c_str(), iconFontSize, &icons_config, icons_ranges);

    // DPI Scale
    if (imguiViewport->DpiScale > 0.0f) {
        ImFontConfig fontConfig;
        fontConfig.SizePixels = 13.0f * imguiViewport->DpiScale;
        io.Fonts->AddFontDefault(&fontConfig);
    }

    descriptorSet_ = ImGui_ImplVulkan_AddTexture(viewport.viewportImage.GetBundle().sampler, viewport.viewportImage.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    plusIcon_ = GenerateIcon(PROJECT_DIR "image/icon/plus.png");
    plusIconDescriptorSet_ = ImGui_ImplVulkan_AddTexture(plusIcon_->GetBundle().sampler, plusIcon_->GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    lightIcon_ = GenerateIcon(PROJECT_DIR "image/icon/light.png");
    lightIconDescriptorSet_ = ImGui_ImplVulkan_AddTexture(lightIcon_->GetBundle().sampler, lightIcon_->GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    cubeIcon_ = GenerateIcon(PROJECT_DIR "image/icon/cube.png");
    cubeIconDescriptorSet_ = ImGui_ImplVulkan_AddTexture(cubeIcon_->GetBundle().sampler, cubeIcon_->GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // TODO: for test
    path = PROJECT_DIR "image/box.png";
    scene.AddResource(path);
    path = PROJECT_DIR "image/earth.jpg";
    scene.AddResource(path);

    dragDropped = false;
}

std::unique_ptr<Image> UI::GenerateIcon(const std::string& filePath)
{
    int width, height, channel;
    vk::DeviceSize imageSize;
    stbi_uc* imageData;

    imageData = stbi_load(filePath.c_str(), &width, &height, &channel, STBI_rgb_alpha);
    imageSize = width * height * 4;

    if (!imageData) {
        spdlog::error("failed to load image");
        return nullptr;
    }

    BufferInput bufferInput = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    Buffer stagingBuffer(bufferInput);
    stagingBuffer.Copy(imageData);

    stbi_image_free(imageData);
    vk::Extent3D extent(width, height, 1);
    std::unique_ptr<Image> image = std::make_unique<Image>();
    image->CreateImage(vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent, vk::ImageTiling::eOptimal);
    image->CreateImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    image->CreateSampler();
    image->SetInfo();

    Command::Begin(commandBuffer_);
    // Set texture image layout to transfer dst optimal
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   *image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTransfer);
    // Copy texture image from staging buffer
    Command::CopyBufferToImage(commandBuffer_,
                               stagingBuffer.GetBundle().buffer,
                               image->GetBundle().image, width,
                               height);
    // Set texture image layout to shader read only
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   *image,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    return std::move(image);
}

void UI::Draw(Scene& scene, Viewport& viewport, size_t frameIndex)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::NewFrame();

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && !ImGui::IsAnyItemHovered()) {
        scene.selectedMeshID = -1;
        scene.selectedMeshInstanceID = -1;
        scene.selectedLightID = -1;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        scene.DeleteMesh();
        scene.DeleteLight();
    }

    DrawDockSpace(scene);
    DrawViewport(scene, viewport, frameIndex);
    DrawListWindow(scene);
    DrawResourceWindow(scene);
    ShowInformationOverlay(scene);

    ImGui::EndFrame();
    ImGui::Render();
}

void UI::DrawDockSpace(Scene& scene)
{
    ImGuiIO& io = ImGui::GetIO();

    static bool p_open = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(imguiViewport->WorkPos);
    ImGui::SetNextWindowSize(imguiViewport->WorkSize);
    ImGui::SetNextWindowViewport(imguiViewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                    ImGuiWindowFlags_NoBackground;

    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &p_open, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Light")) {
                scene.AddLight();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Square")) {
                scene.AddMeshInstance(MESHTYPE::SQUARE);
            }
            if (ImGui::MenuItem("Cube")) {
                scene.AddMeshInstance(MESHTYPE::CUBE);
            }
            if (ImGui::MenuItem("Sphere")) {
                scene.AddMeshInstance(MESHTYPE::SPHERE);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::End();
}

void UI::DrawViewport(Scene& scene, Viewport& viewport, size_t frameIndex)
{
    int width = 0, height = 0;
    glfwGetWindowSize(Window::GetWindow(), &width, &height);
    if (width == 0 || height == 0)
        return;

    Command::Begin(commandBuffer_);
    Command::SetImageMemoryBarrier(commandBuffer_, viewport.viewportImage,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   {},
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    Command::SetImageMemoryBarrier(commandBuffer_, viewport.colorID,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   {},
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    viewport.panelPos = ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMin();
    viewport.panelSize = ImGui::GetContentRegionAvail();
    // debug: drawRect
    // ImGui::GetForegroundDrawList()->AddRect(viewport.panelPos, viewport.panelPos + viewport.panelSize, IM_COL32(255, 0, 0, 255));
    float viewportPanelRatio = viewport.panelSize.x / viewport.panelSize.y;

    if (viewport.panelRatio != viewportPanelRatio || viewport.outDated)
        SetViewportUpToDate(viewport, viewport.panelSize);

    // Set Drag Drop Mouse Position
    ImGui::Image(descriptorSet_, ImVec2{ viewport.panelSize.x, viewport.panelSize.y });
    viewport.isMouseHovered = ImGui::IsItemHovered();
    if (ImGui::BeginDragDropTarget()) {

        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_WINDOW_ITEM", ImGuiDragDropFlags_AcceptBeforeDelivery);
        Resource* data = nullptr;
        if (payload)
            data = (Resource*)payload->Data;

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && data) {

            double mouseX, mouseY;
            glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);
            dragDropMouseX = mouseX;
            dragDropMouseY = mouseY;
            dragDropResource = std::make_unique<Resource>(*data);
            dragDropped = true;
        }

        ImGui::EndDragDropTarget();
    }

    for (auto& light : scene.lights) {

        glm::vec4 pos = scene.camera.GetMatrix().proj * scene.camera.GetMatrix().view * light.model * glm::vec4(light.pos, 1.0f);
        float posZ = pos.z;
        pos /= pos.w;
        pos.x = (pos.x + 1.0f) * 0.5f;
        pos.y = 1.0f - (pos.y + 1.0f) * 0.5f;
        pos.x *= width;
        pos.y *= height;
        ImVec2 screenPos(pos.x, pos.y);
        ImVec2 offset(100, 100);
        offset /= posZ;
        if (posZ > 1.0f && scene.showLightIcon_)
            ImGui::GetWindowDrawList()->AddImage(lightIconDescriptorSet_, viewport.panelPos + screenPos - offset, viewport.panelPos + screenPos + offset, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_BLACK);
    }

    if (scene.selectedMeshID > -1) {
        DrawMeshGuizmo(scene, viewport.panelPos);
    }

    if (scene.selectedLightID > -1) {
        DrawLightGuizmo(scene, viewport.panelPos);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void UI::SetViewportUpToDate(Viewport& viewport, const ImVec2& viewportPanelSize)
{
    viewport.panelRatio = viewportPanelSize.x / viewportPanelSize.y;

    viewport.extent.width = (uint32_t)(viewportPanelSize.x);
    viewport.extent.height = (uint32_t)(viewportPanelSize.y);
#if defined(__APPLE__)
    viewport.extent.width = (uint32_t)(viewportPanelSize.x * ImGui::GetWindowDpiScale());
    viewport.extent.height = (uint32_t)(viewportPanelSize.y * ImGui::GetWindowDpiScale());
#endif

    if (viewport.extent.width == 0 || viewport.extent.height == 0)
        viewport.extent = Swapchain::GetBundle().swapchainImageExtent;

    Device::GetBundle().device.destroyFramebuffer(viewport.framebuffer);
    viewport.DestroyViewportImages();
    viewport.CreateViewportImages();
    viewport.CreateViewportFrameBuffer();
    RecreateViewportDescriptorSets(viewport);

    viewport.outDated = false;
}

void UI::DrawMeshGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.camera.IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.camera.IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.camera.IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPanelPos.x, viewportPanelPos.y, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    float translation[3];
    float rotation[3];
    float scale[3];
    float matrix[16];

    auto& meshInstanceData = scene.meshes[scene.selectedMeshID].instanceData_[scene.selectedMeshInstanceID];
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(meshInstanceData.model), translation, rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
    if (ImGuizmo::Manipulate(glm::value_ptr(scene.camera.GetMatrix().view), glm::value_ptr(scene.camera.GetMatrix().proj), OP, ImGuizmo::LOCAL, matrix)) {
        scene.meshDirtyFlag = true;
    }
    meshInstanceData.model = glm::make_mat4(matrix);
    meshInstanceData.invTranspose = glm::make_mat4(matrix);
    meshInstanceData.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    meshInstanceData.invTranspose = glm::transpose(glm::inverse(meshInstanceData.invTranspose));
}

void UI::DrawLightGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.camera.IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.camera.IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.camera.IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPanelPos.x, viewportPanelPos.y, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    float translation[3];
    float rotation[3];
    float scale[3];
    float matrix[16];

    auto& lightData = scene.lights[scene.selectedLightID];
    glm::mat4 model = glm::translate(lightData.model, lightData.pos);
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
    if (ImGuizmo::Manipulate(glm::value_ptr(scene.camera.GetMatrix().view), glm::value_ptr(scene.camera.GetMatrix().proj), OP, ImGuizmo::LOCAL, matrix)) {
        scene.lightDirtyFlag = true;
    }
    model = glm::translate(glm::make_mat4(matrix), -lightData.pos);
    lightData.model = model;
}

void UI::DrawListWindow(Scene& scene)
{
    // Mesh List
    ImGui::Begin("List");
    ImGui::BeginTabBar("MeshTab");
    if (ImGui::BeginTabItem("Meshes")) {
        if (ImGui::BeginListBox("##Mesh", ImVec2(-FLT_MIN, 0.0f))) {
            for (int i = 0; i < scene.meshes.size(); i++) {
                std::string name(scene.meshes[i].GetName());
                for (int j = 0; j < scene.meshes[i].instanceData_.size(); j++) {
                    ImGui::PushID(i * j + j);
                    if (ImGui::Selectable(name.c_str(), i == scene.selectedMeshID && j == scene.selectedMeshInstanceID)) {
                        scene.selectedMeshID = i;
                        scene.selectedMeshInstanceID = j;
                        scene.selectedLightID = -1;
                    }
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Delete")) {
                            scene.selectedMeshID = i;
                            scene.selectedMeshInstanceID = j;
                            scene.DeleteMesh();
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndListBox();
        }
        // Attributes
        if (scene.selectedMeshID > -1) {

            auto& meshInstanceData = scene.meshes[scene.selectedMeshID].instanceData_[scene.selectedMeshInstanceID];

            float translation[3];
            float rotation[3];
            float scale[3];
            float matrix[16];

            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(meshInstanceData.model), translation, rotation, scale);
            std::vector<std::string> labels = { "Move", "Rotate" };
            ImGui::SeparatorText("Translation");
            ImGui::SliderFloat3(labels[0].append("##translation").c_str(), translation, -10.0f, 10.0f);
            ImGui::SliderFloat3(labels[1].append("##rotation").c_str(), rotation, -180.0f, 180.0f);
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);

            meshInstanceData.model = glm::make_mat4(matrix);
            meshInstanceData.invTranspose = glm::make_mat4(matrix);
            meshInstanceData.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            meshInstanceData.invTranspose = glm::transpose(glm::inverse(meshInstanceData.invTranspose));

            ImGui::SeparatorText("Textures");
            bool useTexture = meshInstanceData.useTexture;
            ImGui::Checkbox("Use Texture", &useTexture);
            meshInstanceData.useTexture = useTexture ? 1 : 0;
        }
        ImGui::EndTabItem();
    }
    // Light List
    if (ImGui::BeginTabItem("Lights")) {
        if (ImGui::BeginListBox("##Light", ImVec2(-FLT_MIN, 0.0f))) {
            for (int i = 0; i < scene.lights.size(); i++) {
                std::string name("light");
                ImGui::PushID(i);
                if (ImGui::Selectable(name.c_str(), i == scene.selectedLightID)) {
                    scene.selectedLightID = i;
                    scene.selectedMeshID = -1;
                }
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) {
                        scene.selectedLightID = i;
                        scene.DeleteLight();
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
            ImGui::Checkbox("Show Light Icon", &scene.showLightIcon_);
            // Attributes
            if (scene.selectedLightID > -1) {

                auto& lightData = scene.lights[scene.selectedLightID];
                glm::mat4 model = glm::translate(lightData.model, lightData.pos);

                float translation[3];
                float rotation[3];
                float scale[3];
                float matrix[16];

                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);
                std::vector<std::string> labels = { "Move", "Rotate", "Color" };
                ImGui::SeparatorText("Translation");
                ImGui::SliderFloat3(labels[0].append("##translation").c_str(), translation, -10.0f, 10.0f);
                ImGui::SliderFloat3(labels[1].append("##rotation").c_str(), rotation, -180.0f, 180.0f);
                ImGui::SliderFloat3(labels[2].append("##color").c_str(), &lightData.color[0], 0.0f, 1.0f);
                ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);

                model = glm::translate(glm::make_mat4(matrix), -lightData.pos);
                lightData.model = model;
            }
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void UI::DrawResourceWindow(Scene& scene)
{
    float desiredButtonSize = 100.0f;
    float padding = desiredButtonSize * 0.4f;
    auto getButtonSize = [desiredButtonSize, padding]() { return desiredButtonSize - padding * 2.0f; };
    float buttonSize = getButtonSize();

    ImGui::Begin("Resources");

    float panelSize = ImGui::GetContentRegionAvail().x;
    int columnCount = std::max(1, (int)(panelSize / desiredButtonSize));
    // Add Resource
    ImGui::Columns(columnCount, 0, false);
    if (ImGui::ImageButton(plusIconDescriptorSet_, { buttonSize, buttonSize }, ImVec2(0, 0), ImVec2(1, 1), (int)padding, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
        std::string path = LaunchNfd({ "Image,Mesh", "jpg,png,obj" });
        if (!path.empty()) {
            scene.AddResource(path);
        }
    }
    ImGui::NextColumn();

    for (auto& resource : scene.resources) {
        if (resource.resourceType == RESOURCETYPE::MESH) {
            ImGui::ImageButton(cubeIconDescriptorSet_, { getButtonSize(), getButtonSize() }, ImVec2(0, 0), ImVec2(1, 1), padding, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
        } else {
            ImGui::ImageButton(static_cast<Texture*>(resource.resource)->descriptorSet, { 100, 100 }, { 0, 0 }, { 1, 1 }, 0);
        }

        // Send Drag Drop
        if (ImGui::BeginDragDropSource()) {
            if (resource.resourceType == RESOURCETYPE::TEXTURE)
                ImGui::Image(static_cast<Texture*>(resource.resource)->descriptorSet, { 100, 100 }, { 0, 0 }, { 1, 1 });
            ImGui::SetDragDropPayload("RESOURCE_WINDOW_ITEM", (void*)&resource, sizeof(resource));
            ImGui::EndDragDropSource();
        }

        ImGui::Text("%s", resource.fileName.c_str());
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::End();
}

void UI::ShowInformationOverlay(const Scene& scene)
{
    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(imguiViewport->Size.x, 0.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("Information", nullptr, window_flags);
    ImGui::Text("%s", GetFrameRate().c_str());
    ImGui::Text("Camera Control: %s [press C]", scene.camera.IsControllable() ? "on" : "off");
    ImGui::End();
}

void UI::RecreateViewportDescriptorSets(const Viewport& viewport)
{
    ImGui_ImplVulkan_RemoveTexture(descriptorSet_);
    descriptorSet_ = ImGui_ImplVulkan_AddTexture(viewport.viewportImage.GetBundle().sampler, viewport.viewportImage.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void UI::AcceptDragDrop(Viewport& viewport, Scene& scene, size_t frameIndex)
{
    if (!dragDropped)
        return;

    const int32_t* pickColor = viewport.PickColor(frameIndex, dragDropMouseX, dragDropMouseY);

    switch (dragDropResource->resourceType) {
    case RESOURCETYPE::MESH:
        scene.AddMeshInstance(static_cast<Mesh*>(dragDropResource->resource)->meshID);
        break;
    case RESOURCETYPE::TEXTURE:
        if (pickColor[0] != -1) {
            auto& instanceData = scene.meshes[pickColor[0]].instanceData_[pickColor[1]];

            instanceData.textureID = static_cast<Texture*>(dragDropResource->resource)->index;
            instanceData.useTexture = true;
            scene.meshDirtyFlag = true;
        }
        break;
    }

    dragDropped = false;
}

UI::~UI()
{
    Device::GetBundle().device.destroyCommandPool(commandPool_);
    for (auto& layout : descriptorSetLayouts_)
        Device::GetBundle().device.destroyDescriptorSetLayout(layout);
    ImGui_ImplVulkan_RemoveTexture(descriptorSet_);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Device::GetBundle().device.destroyDescriptorPool(descriptorPool_);
}
