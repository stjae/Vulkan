#include "ui.h"
#include "font/IconsFontAwesome5.h"

void UI::Setup(const vk::RenderPass& renderPass, Viewport& viewport, Scene& scene)
{
    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffer);

    std::vector<vk::DescriptorPoolSize> poolSizes;
    uint32_t maxSets = 0;

    std::vector<vkn::DescriptorBinding> imGuiBindings;
    imGuiBindings.emplace_back(vk::DescriptorType::eCombinedImageSampler, 100);
    m_descriptorSetLayouts.push_back(vkn::Descriptor::CreateDescriptorSetLayout(imGuiBindings));
    vkn::Descriptor::SetPoolSizes(poolSizes, imGuiBindings, maxSets);

    vkn::Descriptor::CreateDescriptorPool(m_descriptorPool, poolSizes, maxSets, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();

    ImGui_ImplGlfw_InitForVulkan(Window::GetWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vkn::Instance::GetInstance();
    init_info.PhysicalDevice = vkn::Device::Get().physicalDevice;
    init_info.Device = vkn::Device::Get().device;
    init_info.QueueFamily = vkn::Device::Get().graphicsFamilyIndex.value();
    init_info.Queue = vkn::Device::Get().graphicsQueue;
    init_info.DescriptorPool = m_descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = vkn::Swapchain::Get().surfaceCapabilities.minImageCount;
    init_info.ImageCount = vkn::Swapchain::Get().frameImageCount;
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
    std::string path(PROJECT_DIR "font/");
    path.append(FONT_ICON_FILE_NAME_FAS);
    io.Fonts->AddFontFromFileTTF(path.c_str(), iconFontSize, &icons_config, icons_ranges);

    m_viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, viewport.m_image.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkn::Command::Begin(m_commandBuffer);
    m_plusIcon.InsertImage(PROJECT_DIR "image/icon/plus.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_plusIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_plusIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_lightIcon.InsertImage(PROJECT_DIR "image/icon/light.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_lightIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_lightIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_cubeIcon.InsertImage(PROJECT_DIR "image/icon/cube.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_cubeIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_cubeIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_playIcon.InsertImage(PROJECT_DIR "image/icon/play.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_playIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_playIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_stopIcon.InsertImage(PROJECT_DIR "image/icon/stop.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_stopIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_stopIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_commandBuffer.end();

    vkn::Command::SubmitAndWait(m_commandBuffer);

    s_dragDropped = false;
}

void UI::Draw(Scene& scene, Viewport& viewport, const vk::CommandBuffer& commandBuffer)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::NewFrame();

    if (ImGui::IsKeyPressed(ImGuiKey_Q) && !scene.m_camera.IsControllable() || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        scene.Unselect();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        scene.DeleteMeshInstance();
        scene.DeletePointLight();
    }

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, scene.IsPlaying());
    DrawDockSpace(scene);
    DrawViewport(scene, viewport, commandBuffer);
    DrawSceneAttribWindow(scene);
    DrawResourceWindow(scene);
    ShowInformationOverlay(scene);
    ImGui::PopItemFlag();

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
                    scene.m_saveFilePath = openFilePath;
                    SceneSerializer serializer;
                    serializer.Deserialize(scene, openFilePath);
                }
            }
            if (ImGui::MenuItem("Save")) {
                SceneSerializer serializer;
                if (!scene.m_saveFilePath.empty()) {
                    serializer.Serialize(scene, scene.m_saveFilePath);
                } else {
                    std::string saveFilePath = nfdSave({ "Scene", "scn" });
                    scene.m_saveFilePath = saveFilePath;
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
        vk::DescriptorSet icon = scene.m_isPlaying ? m_stopIconDescriptorSet : m_playIconDescriptorSet;
        ImGui::PopItemFlag();
        if (ImGui::ImageButton(icon, { GetIconSize(buttonSize, GetButtonPadding(buttonSize, paddingRatio)), GetIconSize(buttonSize, GetButtonPadding(buttonSize, paddingRatio)) }, { 0, 0 }, { 1, 1 }, GetButtonPadding(buttonSize, paddingRatio))) {
            scene.m_isPlaying = !scene.m_isPlaying;
            if (!scene.m_isPlaying) {
                scene.Stop();
            }
        }
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, scene.IsPlaying());
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

void UI::DrawViewport(Scene& scene, Viewport& viewport, const vk::CommandBuffer& commandBuffer)
{
    int width = 0, height = 0;
    glfwGetWindowSize(Window::GetWindow(), &width, &height);
    if (width == 0 || height == 0)
        return;

    // vkn::Command::Begin(commandBuffer);
    // vkn::Command::SetImageMemoryBarrier(commandBuffer,
    //                                     viewport.m_images[imageIndex].image.Get().image,
    //                                     vk::ImageLayout::eUndefined,
    //                                     vk::ImageLayout::eShaderReadOnlyOptimal,
    //                                     {},
    //                                     vk::AccessFlagBits::eShaderRead,
    //                                     vk::PipelineStageFlagBits::eTopOfPipe,
    //                                     vk::PipelineStageFlagBits::eFragmentShader);
    // vkn::Command::SetImageMemoryBarrier(commandBuffer,
    //                                     viewport.m_images[imageIndex].colorID.Get().image,
    //                                     vk::ImageLayout::eUndefined,
    //                                     vk::ImageLayout::eShaderReadOnlyOptimal,
    //                                     {},
    //                                     vk::AccessFlagBits::eShaderRead,
    //                                     vk::PipelineStageFlagBits::eTopOfPipe,
    //                                     vk::PipelineStageFlagBits::eFragmentShader);
    // commandBuffer.end();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    viewport.m_panelPos = ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMin();
    viewport.m_panelSize = ImGui::GetContentRegionAvail();
    // debugMode: drawRect
    // ImGui::GetForegroundDrawList()->AddRect(viewport.m_panelPos, viewport.m_panelPos + viewport.m_panelSize, IM_COL32(255, 0, 0, 255));
    float viewportPanelRatio = viewport.m_panelSize.x / viewport.m_panelSize.y;

    if (viewport.m_panelRatio != viewportPanelRatio || viewport.m_outDated) {
        vkn::Device::Get().device.waitIdle();
        viewport.UpdateImage();
        RecreateViewportDescriptorSet(viewport);
    }

    // Set Drag Drop Mouse Position
    ImGui::Image(m_viewportImageDescriptorSet, ImVec2{ viewport.m_panelSize.x, viewport.m_panelSize.y });
    viewport.m_isMouseHovered = ImGui::IsItemHovered();
    if (ImGui::BeginDragDropTarget()) {

        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_WINDOW_ITEM", ImGuiDragDropFlags_AcceptBeforeDelivery);
        Resource* data = nullptr;
        if (payload)
            data = (Resource*)payload->Data;

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && data) {

            double mouseX, mouseY;
            glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);
            s_dragDropMouseX = mouseX;
            s_dragDropMouseY = mouseY;
            s_dragDropResource = std::make_unique<Resource>(*data);
            s_dragDropped = true;
        }

        ImGui::EndDragDropTarget();
    }

    for (auto& light : scene.m_pointLights) {

        glm::vec4 pos = scene.m_camera.GetMatrix().proj * scene.m_camera.GetMatrix().view * light.model * glm::vec4(light.pos, 1.0f);
        float posZ = pos.z;
        pos /= pos.w;
        pos.x = (pos.x + 1.0f) * 0.5f;
        pos.y = 1.0f - (pos.y + 1.0f) * 0.5f;
        pos.x *= width;
        pos.y *= height;
        ImVec2 screenPos(pos.x, pos.y);
        ImVec2 offset(100, 100);
        offset /= posZ;
        if (posZ > 1.0f && scene.m_showLightIcon)
            ImGui::GetWindowDrawList()->AddImage(m_lightIconDescriptorSet, viewport.m_panelPos + screenPos - offset, viewport.m_panelPos + screenPos + offset, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_BLACK);
    }

    if (scene.m_selectedMeshID > -1) {
        DrawMeshGuizmo(scene, viewport.m_panelPos);
    }

    if (scene.m_selectedLightID > -1) {
        DrawLightGuizmo(scene, viewport.m_panelPos);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void UI::DrawMeshGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.m_camera.IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.m_camera.IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.m_camera.IsControllable())
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

    auto& mesh = scene.GetSelectedMesh();
    auto& meshInstanceUBO = scene.GetSelectedMeshInstanceUBO();

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(meshInstanceUBO.model), translation, rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
    ImGuizmo::Manipulate(glm::value_ptr(scene.m_camera.GetMatrix().view), glm::value_ptr(scene.m_camera.GetMatrix().proj), OP, ImGuizmo::LOCAL, matrix);
    scene.m_meshDirtyFlag = true;
    meshInstanceUBO.model = glm::make_mat4(matrix);
    meshInstanceUBO.invTranspose = glm::make_mat4(matrix);
    meshInstanceUBO.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    meshInstanceUBO.invTranspose = glm::transpose(glm::inverse(meshInstanceUBO.invTranspose));
}

void UI::DrawLightGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.m_camera.IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.m_camera.IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.m_camera.IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPanelPos.x, viewportPanelPos.y, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    float translation[3];
    float rotation[3];
    float scale[3];
    float matrix[16];

    auto& lightData = scene.m_pointLights[scene.m_selectedLightID];
    glm::mat4 model = glm::translate(lightData.model, lightData.pos);
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
    if (ImGuizmo::Manipulate(glm::value_ptr(scene.m_camera.GetMatrix().view), glm::value_ptr(scene.m_camera.GetMatrix().proj), OP, ImGuizmo::LOCAL, matrix)) {
        scene.m_lightDirtyFlag = true;
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
            for (int i = 0; i < scene.m_meshes.size(); i++) {
                std::string name(scene.m_meshes[i].GetName());
                ImGui::PushID(i);
                if (ImGui::TreeNode(name.c_str())) {
                    for (int j = 0; j < scene.m_meshes[i].GetInstanceCount(); j++) {
                        ImGui::PushID(i * j + j);
                        if (ImGui::Selectable((std::string("instance ") + std::to_string(j)).c_str(), i == scene.m_selectedMeshID && j == scene.m_selectedMeshInstanceID)) {
                            scene.Unselect();
                            scene.m_selectedMeshID = i;
                            scene.m_selectedMeshInstanceID = j;
                        }
                        if (ImGui::BeginPopupContextItem()) {
                            if (ImGui::MenuItem("Delete")) {
                                scene.m_selectedMeshID = i;
                                scene.m_selectedMeshInstanceID = j;
                                scene.DeleteMeshInstance();
                            }
                            ImGui::EndPopup();
                        }
                        ImGui::PopID();
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
        // Mesh Attributes
        if (scene.m_selectedMeshID > -1) {
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
                scene.m_meshDirtyFlag = true;
            if (ImGui::SliderFloat3(labels[1].append("##rotation").c_str(), rotation, -180.0f, 180.0f))
                scene.m_meshDirtyFlag = true;
            if (ImGui::SliderFloat3(labels[2].append("##scale").c_str(), scale, -10.0f, 10.0f))
                scene.m_meshDirtyFlag = true;
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);

            meshInstanceUBO.model = glm::make_mat4(matrix);
            meshInstanceUBO.invTranspose = glm::make_mat4(matrix);
            meshInstanceUBO.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            meshInstanceUBO.invTranspose = glm::transpose(glm::inverse(meshInstanceUBO.invTranspose));

            ImGui::SeparatorText("Texture");
            bool useTexture = meshInstanceUBO.useTexture > 0;
            if (ImGui::Checkbox("Use Texture", &useTexture)) {
                meshInstanceUBO.useTexture = useTexture ? 1 : -1;
                scene.m_meshDirtyFlag = true;
            }

            ImGui::SeparatorText("Material");
            if (ImGui::SliderFloat3("Albedo", &meshInstanceUBO.albedo[0], 0.0f, 1.0f))
                scene.m_meshDirtyFlag = true;
            if (ImGui::SliderFloat("Metallic", &meshInstanceUBO.metallic, 0.0f, 1.0f))
                scene.m_meshDirtyFlag = true;
            if (ImGui::SliderFloat("Roughness", &meshInstanceUBO.roughness, 0.0f, 1.0f))
                scene.m_meshDirtyFlag = true;

            ImGui::SeparatorText("RigidBody");
            ImGui::Text("%s", scene.m_meshes[scene.m_selectedMeshID].GetName().c_str());
            if (!scene.m_meshes[scene.m_selectedMeshID].m_physicsInfo) {
                static MeshPhysicsInfo physicsInfo;
                const char* types[2] = { "Static", "Dynamic" };
                if (ImGui::BeginCombo("Type", types[(int)physicsInfo.rigidBodyType])) {
                    if (ImGui::MenuItem("Static")) {
                        physicsInfo = {};
                        physicsInfo.rigidBodyType = eRigidBodyType::STATIC;
                    }
                    if (ImGui::MenuItem("Dynamic")) {
                        physicsInfo = {};
                        physicsInfo.rigidBodyType = eRigidBodyType::DYNAMIC;
                    }
                    ImGui::EndCombo();
                }
                const char* shapes[6] = { "Box", "Sphere", "Capsule", "Cylinder", "Cone", "Mesh" };
                if (ImGui::BeginCombo("Collider Shape", shapes[(int)physicsInfo.colliderShape])) {
                    if (ImGui::MenuItem("Box"))
                        physicsInfo.colliderShape = eColliderShape::BOX;
                    if (ImGui::MenuItem("Sphere"))
                        physicsInfo.colliderShape = eColliderShape::SPHERE;
                    if (ImGui::MenuItem("Capsule"))
                        physicsInfo.colliderShape = eColliderShape::CAPSULE;
                    if (ImGui::MenuItem("Cylinder"))
                        physicsInfo.colliderShape = eColliderShape::CYLINDER;
                    if (ImGui::MenuItem("Cone"))
                        physicsInfo.colliderShape = eColliderShape::CONE;
                    if (ImGui::MenuItem("Mesh"))
                        physicsInfo.colliderShape = eColliderShape::MESH;
                    ImGui::EndCombo();
                }
                if (ImGui::Button("Add")) {
                    scene.m_meshes[scene.m_selectedMeshID].AddPhysicsInfo(physicsInfo);
                }
            } else {
                // TODO: resize rigidbody
                // resize
                // if (ImGui::SliderFloat3("Size", glm::value_ptr(meshInstanceUBO.physicsInfo->size), 0.0f, 10.0f)) {
                // ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, s, pMatrix);
                // meshInstanceUBO.physicsInfo->matrix = glm::make_mat4(pMatrix);
                // meshInstanceUBO.physicsInfo->scale = glm::make_vec3(scale) * meshInstanceUBO.physicsInfo->size;
                // scene.m_physics.UpdateRigidBody(meshInstanceUBO);
                // }
                if (ImGui::Button("Delete")) {
                    scene.m_meshes[scene.m_selectedMeshID].m_physicsInfo.reset();
                }
            }
        }
        ImGui::EndTabItem();
    }
    // Light List
    if (ImGui::BeginTabItem("Lights")) {
        if (ImGui::BeginListBox("##Light", ImVec2(-FLT_MIN, 0.0f))) {
            for (int i = 0; i < scene.m_pointLights.size(); i++) {
                std::string name("light");
                ImGui::PushID(i);
                if (ImGui::Selectable(name.c_str(), i == scene.m_selectedLightID)) {
                    scene.Unselect();
                }
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) {
                        scene.m_selectedLightID = i;
                        scene.DeletePointLight();
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
            ImGui::Checkbox("Show Light Icon", &scene.m_showLightIcon);
            // Light Attributes
            if (scene.m_selectedLightID > -1) {

                auto& lightData = scene.m_pointLights[scene.m_selectedLightID];
                glm::mat4 model = glm::translate(lightData.model, lightData.pos);

                float translation[3];
                float rotation[3];
                float scale[3];
                float matrix[16];

                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);
                std::vector<std::string> labels = { "Position", "Rotate", "Color" };
                ImGui::SeparatorText("Point Light");
                if (ImGui::SliderFloat3(labels[0].append("##position").c_str(), translation, -10.0f, 10.0f))
                    scene.m_lightDirtyFlag = true;
                if (ImGui::SliderFloat3(labels[2].append("##color").c_str(), &lightData.color[0], 0.0f, 1.0f))
                    scene.m_lightDirtyFlag = true;
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

            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(scene.m_dirLightRot), translation, rotation, scale);
            ImGui::SliderFloat("Near Plane", &scene.m_dirLightNearPlane, 0.0f, 100.0f);
            ImGui::SliderFloat("Far Plane", &scene.m_dirLightFarPlane, 0.0f, 100.0f);
            ImGui::SliderFloat("Size", &scene.m_dirLightSize, 0.0f, 50.0f);
            ImGui::SliderFloat("Distance", &scene.m_dirLightDistance, 0.0f, 100.0f);
            ImGui::SliderFloat3("Rotation", &rotation[0], -1.0f, 1.0f);
            ImGui::SliderFloat3("Color", &scene.m_dirLightUBO.color[0], 0.0f, 1.0f);
            ImGui::SliderFloat("Intensity", &scene.m_dirLightUBO.intensity, 0.0f, 10.0f);
            rotMat = glm::rotate(rotMat, rotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
            rotMat = glm::rotate(rotMat, rotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
            rotMat = glm::rotate(rotMat, rotation[2], glm::vec3(0.0f, 0.0f, 1.0f));
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
            scene.m_dirLightRot = glm::make_mat4(matrix);
            scene.m_dirLightPos = rotMat * glm::vec4(0.0f, scene.m_dirLightDistance, 0.0f, 1.0f);

            // TODO: fix glitch on scroll
            // dir light shadow depth map
            // ImGui_ImplVulkan_RemoveTexture(m_shadowMapDescriptorSet);
            // m_shadowMapDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, scene.m_shadowMap.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            // ImGui::Image(m_shadowMapDescriptorSet, { 200, 200 });
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("IBL")) {
        const float iblButtonSize = 100.0f;
        if (ImGui::ImageButton(m_plusIconDescriptorSet, { GetIconSize(iblButtonSize, GetButtonPadding(iblButtonSize, 0.4f)), GetIconSize(iblButtonSize, GetButtonPadding(iblButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(iblButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
            std::string hdriFilePath = nfdOpen({ "HDRI", "hdr" });
            if (!hdriFilePath.empty()) {
                scene.AddEnvironmentMap(hdriFilePath);
            }
        }
        if (!scene.m_hdriFilePath.empty()) {
            ImGui::Text("%s", scene.m_hdriFilePath.c_str());
        }
        if (ImGui::Button("Remove")) {
            scene.InitHdri();
        }
        ImGui::SeparatorText("Exposure");
        ImGui::SliderFloat("##Exposure", &scene.m_iblExposure, 0.0f, 10.0f);
        if (ImGui::Button("Reset")) {
            scene.m_iblExposure = 1.0f;
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
    if (ImGui::ImageButton(m_plusIconDescriptorSet, { GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)), GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(resourceButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
        std::string path = nfdOpen({ "Model", "gltf,fbx" });
        if (!path.empty()) {
            scene.AddResource(path);
        }
    }
    ImGui::NextColumn();

    for (int i = 0; i < scene.m_resources.size(); i++) {
        ImGui::PushID(i);
        ImGui::ImageButton(m_cubeIconDescriptorSet, { GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)), GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(resourceButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                // TODO: resource deletion
                // scene.m_resources.erase(scene.m_resources.begin() + i);
            }
            ImGui::EndPopup();
        }

        // Send Drag Drop
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("RESOURCE_WINDOW_ITEM", (void*)&scene.m_resources[i], sizeof(scene.m_resources[i]));
            ImGui::EndDragDropSource();
        }

        ImGui::PopID();
        ImGui::Text("%s", scene.m_resources[i].fileName.c_str());
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
    ImGui::Text("Camera Control: %s [press C]", scene.m_camera.IsControllable() ? "on" : "off");
    ImGui::End();
}

void UI::RecreateViewportDescriptorSet(const Viewport& viewport)
{
    ImGui_ImplVulkan_RemoveTexture(m_viewportImageDescriptorSet);
    m_viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, viewport.m_image.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void UI::AcceptDragDrop(Viewport& viewport, Scene& scene)
{
    if (!s_dragDropped)
        return;

    // const int32_t* pickColor = viewport.PickColor(s_dragDropMouseX, s_dragDropMouseY);
    scene.AddMeshInstance(static_cast<Mesh*>(s_dragDropResource->ptr)->GetMeshID());
    s_dragDropped = false;
}

UI::~UI()
{
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    for (auto& layout : m_descriptorSetLayouts)
        vkn::Device::Get().device.destroyDescriptorSetLayout(layout);
    ImGui_ImplVulkan_RemoveTexture(m_viewportImageDescriptorSet);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkn::Device::Get().device.destroyDescriptorPool(m_descriptorPool);
}
