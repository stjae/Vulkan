#include "ui.h"
#include "font/IconsFontAwesome5.h"

void UI::Setup(const vk::RenderPass& renderPass, Viewport& viewport, Scene& scene)
{
    vkn::Command::CreateCommandPool(commandPool_);
    vkn::Command::AllocateCommandBuffer(commandPool_, commandBuffer_);

    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> imGuiBindings;
    imGuiBindings.emplace_back(0, vk::DescriptorType::eCombinedImageSampler, 100);
    descriptorSetLayouts_.push_back(vkn::Descriptor::CreateDescriptorSetLayout(imGuiBindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, imGuiBindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(descriptorPool_, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();

    ImGui_ImplGlfw_InitForVulkan(Window::GetWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vkn::Instance::GetBundle().instance;
    init_info.PhysicalDevice = vkn::Device::GetBundle().physicalDevice;
    init_info.Device = vkn::Device::GetBundle().device;
    init_info.QueueFamily = vkn::Device::GetBundle().graphicsFamilyIndex.value();
    init_info.Queue = vkn::Device::GetBundle().graphicsQueue;
    init_info.DescriptorPool = descriptorPool_;
    init_info.Subpass = 0;
    init_info.MinImageCount = vkn::Swapchain::surfaceCapabilities.minImageCount;
    init_info.ImageCount = vkn::Swapchain::GetBundle().frameImageCount;
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

    descriptorSet_ = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, viewport.viewportImage.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    plusIcon_.InsertImage(PROJECT_DIR "image/icon/plus.png", vk::Format::eR8G8B8A8Srgb, commandBuffer_);
    vkn::Command::Submit(&commandBuffer_, 1);
    plusIconDescriptorSet_ = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, plusIcon_.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    lightIcon_.InsertImage(PROJECT_DIR "image/icon/light.png", vk::Format::eR8G8B8A8Srgb, commandBuffer_);
    vkn::Command::Submit(&commandBuffer_, 1);
    lightIconDescriptorSet_ = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, lightIcon_.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    cubeIcon_.InsertImage(PROJECT_DIR "image/icon/cube.png", vk::Format::eR8G8B8A8Srgb, commandBuffer_);
    vkn::Command::Submit(&commandBuffer_, 1);
    cubeIconDescriptorSet_ = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, cubeIcon_.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    playIcon_.InsertImage(PROJECT_DIR "image/icon/play.png", vk::Format::eR8G8B8A8Srgb, commandBuffer_);
    vkn::Command::Submit(&commandBuffer_, 1);
    playIconDescriptorSet_ = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, playIcon_.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    stopIcon_.InsertImage(PROJECT_DIR "image/icon/stop.png", vk::Format::eR8G8B8A8Srgb, commandBuffer_);
    vkn::Command::Submit(&commandBuffer_, 1);
    stopIconDescriptorSet_ = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, stopIcon_.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    dragDropped = false;
}

void UI::Draw(Scene& scene, Viewport& viewport, size_t frameIndex)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::NewFrame();

    if (ImGui::IsKeyPressed(ImGuiKey_Q) && !scene.camera_.IsControllable() || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        scene.selectedMeshID_ = -1;
        scene.selectedMeshInstanceID_ = -1;
        scene.selectedLightID_ = -1;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        scene.DeleteMeshInstance();
        scene.DeletePointLight();
    }

    DrawDockSpace(scene);
    DrawViewport(scene, viewport, frameIndex);
    DrawSceneAttribWindow(scene);
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
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 10, 10 });
    ImGui::Begin("DockSpace", &p_open, window_flags);
    ImGui::PopStyleVar(4);

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    // Top Menu Bar
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 10, 10 });
    if (ImGui::BeginMenuBar()) {
        bool openNewScene = false;
        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("New")) {
                openNewScene = true;
            }
            if (ImGui::MenuItem("Open")) {
                std::string openFilePath = nfdOpen({ "Scene", "scn" });
                if (!openFilePath.empty()) {
                    scene.saveFilePath_ = openFilePath;
                    SceneSerializer serializer;
                    serializer.Deserialize(scene, openFilePath);
                }
            }
            if (ImGui::MenuItem("Save")) {
                SceneSerializer serializer;
                if (!scene.saveFilePath_.empty()) {
                    serializer.Serialize(scene, scene.saveFilePath_);
                } else {
                    std::string saveFilePath = nfdSave({ "Scene", "scn" });
                    scene.saveFilePath_ = saveFilePath;
                    serializer.Serialize(scene, saveFilePath);
                }
            }
            if (ImGui::MenuItem("Save as")) {
                std::string saveFilePath = nfdSave({ "Scene", "scn" });
                SceneSerializer serializer;
                serializer.Serialize(scene, saveFilePath);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::MenuItem("Point Light")) {
                scene.AddLight();
            }
            ImGui::EndMenu();
        }
        const float buttonSize = ImGui::GetCurrentWindow()->MenuBarRect().GetSize().y;
        const float paddingRatio = 0.2f;
        ImGui::SetCursorPosX(ImGui::GetCurrentWindow()->MenuBarRect().GetSize().x * 0.5f - GetIconSize(buttonSize, GetButtonPadding(buttonSize, paddingRatio)));
        // ImGui::GetForegroundDrawList()->AddRect(ImGui::GetWindowPos(), ImGui::GetCurrentWindow()->MenuBarRect().GetSize() * 0.5f - ImVec2(GetIconSize(buttonSize, GetButtonPadding(buttonSize, paddingRatio)), 0), IM_COL32(255, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        vk::DescriptorSet icon = scene.isPlaying_ ? stopIconDescriptorSet_ : playIconDescriptorSet_;
        if (ImGui::ImageButton(icon, { GetIconSize(buttonSize, GetButtonPadding(buttonSize, paddingRatio)), GetIconSize(buttonSize, GetButtonPadding(buttonSize, paddingRatio)) }, { 0, 0 }, { 1, 1 }, GetButtonPadding(buttonSize, paddingRatio))) {
            scene.isPlaying_ = !scene.isPlaying_;
            if (!scene.isPlaying_) {
                scene.Stop();
            }
        }
        ImGui::PopStyleColor(3);
        ImGui::EndMenuBar();

        // open new scene popup modal
        if (openNewScene)
            ImGui::OpenPopup("NewScene");

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("NewScene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure to open a new scene?\nUnsaved work will be lost.");
            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PopStyleVar();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                scene.InitScene();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::PopStyleVar();
    ImGui::End();
}

void UI::DrawViewport(Scene& scene, Viewport& viewport, size_t frameIndex)
{
    int width = 0, height = 0;
    glfwGetWindowSize(Window::GetWindow(), &width, &height);
    if (width == 0 || height == 0)
        return;

    vkn::Command::Begin(commandBuffer_);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        viewport.viewportImage.GetBundle().image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        {},
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eTopOfPipe,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        viewport.colorID.GetBundle().image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        {},
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eTopOfPipe,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    vkn::Command::Submit(&commandBuffer_, 1);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    viewport.panelPos = ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMin();
    viewport.panelSize = ImGui::GetContentRegionAvail();
    // debugMode: drawRect
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

    for (auto& light : scene.pointLights_) {

        glm::vec4 pos = scene.camera_.GetMatrix().proj * scene.camera_.GetMatrix().view * light.model * glm::vec4(light.pos, 1.0f);
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

    if (scene.selectedMeshID_ > -1) {
        DrawMeshGuizmo(scene, viewport.panelPos);
    }

    if (scene.selectedLightID_ > -1) {
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
        viewport.extent = vkn::Swapchain::GetBundle().swapchainImageExtent;

    vkn::Device::GetBundle().device.destroyFramebuffer(viewport.framebuffer);
    viewport.DestroyViewportImages();
    viewport.CreateViewportImages();
    viewport.CreateViewportFrameBuffer();
    RecreateViewportDescriptorSets(viewport);

    viewport.outDated = false;
}

void UI::DrawMeshGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.camera_.IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.camera_.IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.camera_.IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPanelPos.x, viewportPanelPos.y, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    float translation[3];
    float rotation[3];
    float scale[3];
    float matrix[16];
    float pMatrix[16];

    auto& meshInstanceUBO = scene.GetSelectedMeshInstanceUBO();

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(meshInstanceUBO.model), translation, rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
    if (ImGuizmo::Manipulate(glm::value_ptr(scene.camera_.GetMatrix().view), glm::value_ptr(scene.camera_.GetMatrix().proj), OP, ImGuizmo::LOCAL, matrix)) {
        if (meshInstanceUBO.pInfo) {
            float s[3] = { 1.0f, 1.0f, 1.0f };
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, s, pMatrix);
            meshInstanceUBO.pInfo->matrix = glm::make_mat4(pMatrix);
            meshInstanceUBO.pInfo->scale = glm::make_vec3(scale);
            scene.physics_.UpdateRigidBody(meshInstanceUBO);
        }
    }
    scene.meshDirtyFlag_ = true;
    meshInstanceUBO.model = glm::make_mat4(matrix);
    meshInstanceUBO.invTranspose = glm::make_mat4(matrix);
    meshInstanceUBO.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    meshInstanceUBO.invTranspose = glm::transpose(glm::inverse(meshInstanceUBO.invTranspose));
}

void UI::DrawLightGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.camera_.IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.camera_.IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.camera_.IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPanelPos.x, viewportPanelPos.y, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    float translation[3];
    float rotation[3];
    float scale[3];
    float matrix[16];

    auto& lightData = scene.pointLights_[scene.selectedLightID_];
    glm::mat4 model = glm::translate(lightData.model, lightData.pos);
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
    if (ImGuizmo::Manipulate(glm::value_ptr(scene.camera_.GetMatrix().view), glm::value_ptr(scene.camera_.GetMatrix().proj), OP, ImGuizmo::LOCAL, matrix)) {
        scene.lightDirtyFlag_ = true;
    }
    model = glm::translate(glm::make_mat4(matrix), -lightData.pos);
    lightData.model = model;
}

void UI::DrawSceneAttribWindow(Scene& scene)
{
    ImGui::Begin("Scene");
    ImGui::BeginTabBar("MeshTab");
    // Mesh List
    if (ImGui::BeginTabItem("Meshes")) {
        if (ImGui::BeginListBox("##Mesh", ImVec2(-FLT_MIN, 0.0f))) {
            for (int i = 0; i < scene.meshes_.size(); i++) {
                std::string name(scene.meshes_[i].GetName());
                for (int j = 0; j < scene.meshes_[i].GetInstanceCount(); j++) {
                    ImGui::PushID(i * j + j);
                    if (ImGui::Selectable(name.c_str(), i == scene.selectedMeshID_ && j == scene.selectedMeshInstanceID_)) {
                        scene.selectedMeshID_ = i;
                        scene.selectedMeshInstanceID_ = j;
                        scene.selectedLightID_ = -1;
                    }
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Delete")) {
                            scene.selectedMeshID_ = i;
                            scene.selectedMeshInstanceID_ = j;
                            scene.DeleteMeshInstance();
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndListBox();
        }
        // Mesh Attributes
        if (scene.selectedMeshID_ > -1) {
            auto& meshInstanceUBO = scene.GetSelectedMeshInstanceUBO();

            float translation[3];
            float rotation[3];
            float scale[3];
            float matrix[16];
            float pMatrix[16];

            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(meshInstanceUBO.model), translation, rotation, scale);
            std::vector<std::string> labels = { "Move", "Rotate", "Scale" };
            ImGui::SeparatorText("Translation");
            if (ImGui::SliderFloat3(labels[0].append("##translation").c_str(), translation, -10.0f, 10.0f))
                scene.meshDirtyFlag_ = true;
            if (ImGui::SliderFloat3(labels[1].append("##rotation").c_str(), rotation, -180.0f, 180.0f))
                scene.meshDirtyFlag_ = true;
            if (ImGui::SliderFloat3(labels[2].append("##scale").c_str(), scale, -10.0f, 10.0f))
                scene.meshDirtyFlag_ = true;
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);

            meshInstanceUBO.model = glm::make_mat4(matrix);
            meshInstanceUBO.invTranspose = glm::make_mat4(matrix);
            meshInstanceUBO.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            meshInstanceUBO.invTranspose = glm::transpose(glm::inverse(meshInstanceUBO.invTranspose));

            ImGui::SeparatorText("Texture");
            bool useTexture = meshInstanceUBO.useTexture > 0;
            if (ImGui::Checkbox("Use Texture", &useTexture)) {
                meshInstanceUBO.useTexture = useTexture ? 1 : -1;
                scene.meshDirtyFlag_ = true;
            }

            ImGui::SeparatorText("Material");
            if (ImGui::SliderFloat3("Albedo", &meshInstanceUBO.albedo[0], 0.0f, 1.0f))
                scene.meshDirtyFlag_ = true;
            if (ImGui::SliderFloat("Metallic", &meshInstanceUBO.metallic, 0.0f, 1.0f))
                scene.meshDirtyFlag_ = true;
            if (ImGui::SliderFloat("Roughness", &meshInstanceUBO.roughness, 0.0f, 1.0f))
                scene.meshDirtyFlag_ = true;

            ImGui::SeparatorText("RigidBody");
            if (!meshInstanceUBO.pInfo) {
                static MeshInstancePhysicsInfo previewInfo;
                const char* types[2] = { "Static", "Dynamic" };
                if (ImGui::BeginCombo("Type", types[(int)previewInfo.rigidBodyType])) {
                    if (ImGui::MenuItem("Static"))
                        previewInfo.rigidBodyType = eRigidBodyType::STATIC;
                    if (ImGui::MenuItem("Dynamic"))
                        previewInfo.rigidBodyType = eRigidBodyType::DYNAMIC;
                    ImGui::EndCombo();
                }
                const char* shapes[5] = { "Box", "Sphere", "Capsule", "Cylinder", "Cone" };
                if (ImGui::BeginCombo("Collider Shape", shapes[(int)previewInfo.rigidBodyShape])) {
                    if (ImGui::MenuItem("Box"))
                        previewInfo.rigidBodyShape = eRigidBodyShape::BOX;
                    if (ImGui::MenuItem("Sphere"))
                        previewInfo.rigidBodyShape = eRigidBodyShape::SPHERE;
                    if (ImGui::MenuItem("Capsule"))
                        previewInfo.rigidBodyShape = eRigidBodyShape::CAPSULE;
                    if (ImGui::MenuItem("Cylinder"))
                        previewInfo.rigidBodyShape = eRigidBodyShape::CYLINDER;
                    if (ImGui::MenuItem("Cone"))
                        previewInfo.rigidBodyShape = eRigidBodyShape::CONE;
                    ImGui::EndCombo();
                }
                if (ImGui::Button("Add")) {
                    float s[3] = { 1.0f, 1.0f, 1.0f };
                    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, s, pMatrix);
                    previewInfo.matrix = glm::make_mat4(pMatrix);
                    previewInfo.scale = glm::make_vec3(scale);
                    scene.physics_.AddRigidBody(meshInstanceUBO, previewInfo);
                }
            } else {
                // resize
                if (ImGui::SliderFloat3("Size", glm::value_ptr(meshInstanceUBO.pInfo->size), 0.0f, 10.0f)) {
                    float s[3] = { 1.0f, 1.0f, 1.0f };
                    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, s, pMatrix);
                    meshInstanceUBO.pInfo->matrix = glm::make_mat4(pMatrix);
                    meshInstanceUBO.pInfo->scale = glm::make_vec3(scale) * meshInstanceUBO.pInfo->size;
                    scene.physics_.UpdateRigidBody(meshInstanceUBO);
                }
                if (ImGui::Button("Delete")) {
                    scene.physics_.DeleteRigidBody(meshInstanceUBO);
                }
            }
        }
        ImGui::EndTabItem();
    }
    // Light List
    if (ImGui::BeginTabItem("Lights")) {
        if (ImGui::BeginListBox("##Light", ImVec2(-FLT_MIN, 0.0f))) {
            for (int i = 0; i < scene.pointLights_.size(); i++) {
                std::string name("light");
                ImGui::PushID(i);
                if (ImGui::Selectable(name.c_str(), i == scene.selectedLightID_)) {
                    scene.selectedLightID_ = i;
                    scene.selectedMeshID_ = -1;
                }
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) {
                        scene.selectedLightID_ = i;
                        scene.DeletePointLight();
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
            ImGui::Checkbox("Show Light Icon", &scene.showLightIcon_);
            // Light Attributes
            if (scene.selectedLightID_ > -1) {

                auto& lightData = scene.pointLights_[scene.selectedLightID_];
                glm::mat4 model = glm::translate(lightData.model, lightData.pos);

                float translation[3];
                float rotation[3];
                float scale[3];
                float matrix[16];

                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);
                std::vector<std::string> labels = { "Position", "Rotate", "Color" };
                ImGui::SeparatorText("Point Light");
                if (ImGui::SliderFloat3(labels[0].append("##position").c_str(), translation, -10.0f, 10.0f))
                    scene.lightDirtyFlag_ = true;
                if (ImGui::SliderFloat3(labels[2].append("##color").c_str(), &lightData.color[0], 0.0f, 1.0f))
                    scene.lightDirtyFlag_ = true;
                ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);

                model = glm::translate(glm::make_mat4(matrix), -lightData.pos);
                lightData.model = model;
            }
            // Directional Light
            ImGui::SeparatorText("Directional Light");

            glm::mat4 rotMat(1.0f);
            float translation[3];
            float rotation[3];
            float scale[3];
            float matrix[16];

            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(scene.dirLightRot_), translation, rotation, scale);
            ImGui::SliderFloat("Near Plane", &scene.dirLightNearPlane_, 0.0f, 100.0f);
            ImGui::SliderFloat("Far Plane", &scene.dirLightFarPlane_, 0.0f, 100.0f);
            ImGui::SliderFloat("Size", &scene.dirLightSize_, 0.0f, 50.0f);
            ImGui::SliderFloat("Distance", &scene.dirLightDistance_, 0.0f, 100.0f);
            ImGui::SliderFloat3("Rotation", &rotation[0], -1.0f, 1.0f);
            ImGui::SliderFloat3("Color", &scene.dirLightUBO_.color[0], 0.0f, 1.0f);
            ImGui::SliderFloat("Intensity", &scene.dirLightUBO_.intensity, 0.0f, 10.0f);
            rotMat = glm::rotate(rotMat, rotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
            rotMat = glm::rotate(rotMat, rotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
            rotMat = glm::rotate(rotMat, rotation[2], glm::vec3(0.0f, 0.0f, 1.0f));
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
            scene.dirLightRot_ = glm::make_mat4(matrix);
            scene.dirLightPos_ = rotMat * glm::vec4(0.0f, scene.dirLightDistance_, 0.0f, 1.0f);

            // TODO: fix glitch on scroll
            // dir light shadow depth map
            // ImGui_ImplVulkan_RemoveTexture(shadowMapDescriptorSet_);
            // shadowMapDescriptorSet_ = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, scene.shadowMap_.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            // ImGui::Image(shadowMapDescriptorSet_, { 200, 200 });
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("IBL")) {
        const float iblButtonSize = 100.0f;
        if (ImGui::ImageButton(plusIconDescriptorSet_, { GetIconSize(iblButtonSize, GetButtonPadding(iblButtonSize, 0.4f)), GetIconSize(iblButtonSize, GetButtonPadding(iblButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(iblButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
            std::string hdriFilePath = nfdOpen({ "HDRI", "hdr" });
            if (!hdriFilePath.empty()) {
                scene.AddEnvironmentMap(hdriFilePath);
            }
        }
        if (!scene.hdriFilePath_.empty()) {
            ImGui::Text("%s", scene.hdriFilePath_.c_str());
        }
        if (ImGui::Button("Remove")) {
            scene.InitHdri();
        }
        ImGui::SeparatorText("Exposure");
        ImGui::SliderFloat("##Exposure", &scene.iblExposure_, 0.0f, 10.0f);
        if (ImGui::Button("Reset")) {
            scene.iblExposure_ = 1.0f;
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void UI::DrawResourceWindow(Scene& scene)
{
    ImGui::Begin("Resources");

    const float resourceButtonSize = 100.0f;
    float panelSize = ImGui::GetContentRegionAvail().x;
    int columnCount = std::max(1, (int)(panelSize / resourceButtonSize));
    // Add Resource
    ImGui::Columns(columnCount, nullptr, false);
    if (ImGui::ImageButton(plusIconDescriptorSet_, { GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)), GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(resourceButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
        std::string path = nfdOpen({ "Model", "gltf,fbx" });
        if (!path.empty()) {
            scene.AddResource(path);
        }
    }
    ImGui::NextColumn();

    for (int i = 0; i < scene.resources_.size(); i++) {
        ImGui::PushID(i);
        ImGui::ImageButton(cubeIconDescriptorSet_, { GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)), GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(resourceButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                // TODO: resource deletion
                // scene.resources_.erase(scene.resources_.begin() + i);
            }
            ImGui::EndPopup();
        }

        // Send Drag Drop
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("RESOURCE_WINDOW_ITEM", (void*)&scene.resources_[i], sizeof(scene.resources_[i]));
            ImGui::EndDragDropSource();
        }

        ImGui::PopID();
        ImGui::Text("%s", scene.resources_[i].fileName.c_str());
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
    ImGui::Text("Camera Control: %s [press C]", scene.camera_.IsControllable() ? "on" : "off");
    ImGui::End();
}

void UI::RecreateViewportDescriptorSets(const Viewport& viewport)
{
    ImGui_ImplVulkan_RemoveTexture(descriptorSet_);
    descriptorSet_ = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, viewport.viewportImage.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void UI::AcceptDragDrop(Viewport& viewport, Scene& scene, size_t frameIndex)
{
    if (!dragDropped)
        return;

    // const int32_t* pickColor = viewport.PickColor(dragDropMouseX, dragDropMouseY);
    scene.AddMeshInstance(static_cast<Mesh*>(dragDropResource->ptr)->GetMeshID());
    dragDropped = false;
}

UI::~UI()
{
    vkn::Device::GetBundle().device.destroyCommandPool(commandPool_);
    for (auto& layout : descriptorSetLayouts_)
        vkn::Device::GetBundle().device.destroyDescriptorSetLayout(layout);
    ImGui_ImplVulkan_RemoveTexture(descriptorSet_);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkn::Device::GetBundle().device.destroyDescriptorPool(descriptorPool_);
}
