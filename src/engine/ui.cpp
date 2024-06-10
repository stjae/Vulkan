#include "ui.h"
#include "font/IconsFontAwesome5.h"

void UI::Setup(const vk::RenderPass& renderPass, Viewport& viewport, Scene& scene, const vk::CommandBuffer& commandBuffer)
{
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

    m_plusIcon.InsertImage(PROJECT_DIR "image/icon/plus.png", vk::Format::eR8G8B8A8Srgb, commandBuffer);
    m_plusIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_plusIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_lightIcon.InsertImage(PROJECT_DIR "image/icon/lightbulb.png", vk::Format::eR8G8B8A8Srgb, commandBuffer);
    m_lightIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_lightIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_cubeIcon.InsertImage(PROJECT_DIR "image/icon/cube.png", vk::Format::eR8G8B8A8Srgb, commandBuffer);
    m_cubeIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_cubeIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_playIcon.InsertImage(PROJECT_DIR "image/icon/play.png", vk::Format::eR8G8B8A8Srgb, commandBuffer);
    m_playIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_playIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_stopIcon.InsertImage(PROJECT_DIR "image/icon/stop.png", vk::Format::eR8G8B8A8Srgb, commandBuffer);
    m_stopIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_stopIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    s_dragDropped = false;
}

void UI::Draw(Scene& scene, Viewport& viewport, bool& init, const vk::CommandBuffer& commandBuffer)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::NewFrame();

    if (ImGui::IsKeyPressed(ImGuiKey_Q) && !scene.m_mainCamera->IsControllable() || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        scene.UnselectAll();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && scene.m_selectedMeshID > -1 && scene.m_selectedMeshInstanceID > -1) {
        scene.DeleteMeshInstance(scene.GetSelectedMesh(), scene.GetSelectedMeshInstance());
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && scene.m_selectedLightIndex > -1) {
        scene.DeletePointLight(commandBuffer);
    }

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, scene.IsPlaying());
    DrawDockSpace(scene, init, commandBuffer);
    DrawViewport(scene, viewport);
    DrawSceneAttribWindow(scene, commandBuffer);
    DrawResourceWindow(scene, commandBuffer);
    ShowInformationOverlay(scene);
    ImGui::PopItemFlag();

    ImGui::EndFrame();
    ImGui::Render();
}

void UI::DrawInitPopup(bool& init, Scene& scene, const vk::CommandBuffer& commandBuffer)
{
    ImGui::OpenPopup("Welcome");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Welcome", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::Button("New Scene", ImVec2(120, 0))) {
            std::string saveFolderPath = nfdPickFolder();
            scene.m_sceneFolderPath = saveFolderPath;
            if (!saveFolderPath.empty()) {
                init = false;
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::Button("Open Scene", ImVec2(120, 0))) {
            std::string openFilePath = nfdOpen({ "Scene", "scn" });
            if (!openFilePath.empty()) {
                SceneSerializer serializer;
                serializer.Deserialize(scene, openFilePath, commandBuffer);
                init = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

void UI::DrawDockSpace(Scene& scene, bool& init, const vk::CommandBuffer& commandBuffer)
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
                std::string sceneFilePath = nfdOpen({ "Scene", "scn" });
                if (!sceneFilePath.empty()) {
                    scene.m_sceneFilePath = sceneFilePath;
                    SceneSerializer serializer;
                    serializer.Deserialize(scene, sceneFilePath, commandBuffer);
                }
            }
            if (ImGui::MenuItem("Save")) {
                SceneSerializer serializer;
                if (!scene.m_sceneFilePath.empty()) {
                    serializer.Serialize(scene);
                } else {
                    std::string sceneFilePath = nfdSave({ "Scene", "scn" });
                    scene.m_sceneFolderPath = sceneFilePath.substr(0, sceneFilePath.find_last_of("/\\"));
                    scene.m_sceneFilePath = sceneFilePath;
                    serializer.Serialize(scene);
                }
            }
            if (ImGui::MenuItem("Save as")) {
                std::string sceneFilePath = nfdSave({ "Scene", "scn" });
                scene.m_sceneFolderPath = sceneFilePath.substr(0, sceneFilePath.find_last_of("/\\"));
                scene.m_sceneFilePath = sceneFilePath;
                SceneSerializer serializer;
                serializer.Serialize(scene);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add")) {
            if (ImGui::MenuItem("Point Light")) {
                scene.AddPointLight(commandBuffer);
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
                scene.Stop(commandBuffer);
            }
        }
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, scene.IsPlaying());
        ImGui::PopStyleColor(3);
        ImGui::EndMenuBar();

        // open new scene popup modal
        if (openNewScene)
            ImGui::OpenPopup("New Scene");

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("New Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure to open a new scene?\nUnsaved work will be lost.");
            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PopStyleVar();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                scene.Clear(commandBuffer);
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (init) {
            DrawInitPopup(init, scene, commandBuffer);
        }
    }
    ImGui::PopStyleVar();
    ImGui::End();
}

void UI::DrawViewport(Scene& scene, Viewport& viewport)
{
    int width = 0, height = 0;
    glfwGetWindowSize(Window::GetWindow(), &width, &height);
    if (width == 0 || height == 0)
        return;

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

    for (auto& light : scene.m_pointLight.m_UBOs) {
        glm::vec4 pos = scene.m_mainCamera->GetUBO().proj * scene.m_mainCamera->GetUBO().view * glm::vec4(light.pos, 1.0f);
        float posZ = pos.z;
        pos /= pos.w;
        pos.x = (pos.x + 1.0f) * 0.5f;
        pos.y = 1.0f - (pos.y + 1.0f) * 0.5f;
        pos.x *= width;
        pos.y *= height;
        ImVec2 screenPos(pos.x, pos.y);
        ImVec2 offset(300, 300);
        offset /= posZ;
        if (posZ > 1.0f && scene.m_showLightIcon && scene.m_selectedCameraUUID == 0)
            ImGui::GetWindowDrawList()->AddImage(m_lightIconDescriptorSet, viewport.m_panelPos + screenPos - offset, viewport.m_panelPos + screenPos + offset, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_BLACK);
    }

    if (scene.m_selectedMeshID > -1) {
        DrawMeshGuizmo(scene, viewport.m_panelPos);
    }

    if (scene.m_selectedLightIndex > -1) {
        DrawLightGuizmo(scene, viewport.m_panelPos);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void UI::DrawMeshGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.m_mainCamera->IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.m_mainCamera->IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.m_mainCamera->IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPanelPos.x, viewportPanelPos.y, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    auto& mesh = scene.GetSelectedMesh();
    auto& meshInstance = scene.GetSelectedMeshInstance();

    if (ImGuizmo::Manipulate(glm::value_ptr(scene.m_mainCamera->GetUBO().view), glm::value_ptr(scene.m_mainCamera->GetUBO().proj), OP, ImGuizmo::LOCAL, glm::value_ptr(meshInstance.UBO.model))) {
        meshInstance.UpdateTransform();
        mesh.UpdateUBO(meshInstance);
    }
}

void UI::DrawLightGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.m_mainCamera->IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.m_mainCamera->IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.m_mainCamera->IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPanelPos.x, viewportPanelPos.y, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    auto& light = scene.m_pointLight.m_UBOs[scene.m_selectedLightIndex];
    glm::mat4 lightTranslation = glm::translate(glm::mat4(1.0f), light.pos);
    ImGuizmo::Manipulate(glm::value_ptr(scene.m_mainCamera->GetUBO().view), glm::value_ptr(scene.m_mainCamera->GetUBO().proj), OP, ImGuizmo::LOCAL, glm::value_ptr(lightTranslation));
    light.pos = lightTranslation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void UI::DrawSceneAttribWindow(Scene& scene, const vk::CommandBuffer& commandBuffer)
{
    ImGui::Begin("Scene");
    ImGui::BeginTabBar("Scene");
    // Mesh List
    if (ImGui::BeginTabItem("Mesh")) {
        if (ImGui::BeginListBox("##Mesh", ImVec2(-FLT_MIN, 0.0f))) {
            for (int i = 0; i < scene.m_meshes.size(); i++) {
                std::string name(scene.m_meshes[i]->GetName());
                ImGui::PushID(i);
                if (ImGui::TreeNode(name.c_str())) {
                    for (int j = 0; j < scene.m_meshes[i]->GetInstanceCount(); j++) {
                        ImGui::PushID(i * j + j);
                        if (ImGui::Selectable((std::string("instance ") + std::to_string(j)).c_str(), i == scene.m_selectedMeshID && j == scene.m_selectedMeshInstanceID)) {
                            scene.UnselectAll();
                            scene.m_selectedMeshID = i;
                            scene.m_selectedMeshInstanceID = j;
                        }
                        if (ImGui::BeginPopupContextItem()) {
                            if (ImGui::MenuItem("Delete")) {
                                scene.m_selectedMeshID = i;
                                scene.m_selectedMeshInstanceID = j;
                                scene.DeleteMeshInstance(scene.GetSelectedMesh(), scene.GetSelectedMeshInstance());
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
            auto& mesh = scene.GetSelectedMesh();
            auto& meshInstance = scene.GetSelectedMeshInstance();

            ImGui::SeparatorText((std::string("ID: ") + std::to_string(meshInstance.UUID)).c_str());
            ImGui::Separator();
            ImGui::DragFloat3("Translation##_INSTANCE", &meshInstance.translation[0], 0.1f);
            ImGui::DragFloat3("Rotation##_INSTANCE", &meshInstance.rotation[0], 0.1f);
            ImGui::DragFloat3("Scale##_INSTANCE", &meshInstance.scale[0], 0.1f);
            meshInstance.UpdateMatrix();
            mesh.UpdateUBO(meshInstance);

            ImGui::SeparatorText("Texture");
            bool useTexture = meshInstance.UBO.useTexture > 0;
            if (ImGui::Checkbox("Use Texture", &useTexture)) {
                meshInstance.UBO.useTexture = useTexture ? 1 : -1;
                mesh.UpdateUBO(meshInstance);
            }

            ImGui::SeparatorText("Material");
            if (ImGui::SliderFloat3("Albedo", &meshInstance.UBO.albedo[0], 0.0f, 1.0f))
                mesh.UpdateUBO(meshInstance);
            if (ImGui::SliderFloat("Metallic", &meshInstance.UBO.metallic, 0.0f, 1.0f))
                mesh.UpdateUBO(meshInstance);
            if (ImGui::SliderFloat("Roughness", &meshInstance.UBO.roughness, 0.0f, 1.0f))
                mesh.UpdateUBO(meshInstance);

            ImGui::SeparatorText("RigidBody");
            ImGui::Text("%s", scene.m_meshes[scene.m_selectedMeshID]->GetName().c_str());
            if (!meshInstance.physicsInfo) {
                static PhysicsInfo physicsInfo;
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
                    if (physicsInfo.rigidBodyType == eRigidBodyType::STATIC) {
                        if (ImGui::MenuItem("Mesh"))
                            physicsInfo.colliderShape = eColliderShape::MESH;
                    }
                    ImGui::EndCombo();
                }
                if (ImGui::Button("Add##physics")) {
                    scene.AddPhysics(*scene.m_meshes[scene.m_selectedMeshID], meshInstance, physicsInfo, commandBuffer);
                }
            } else {
                if (ImGui::DragFloat3("Scale##physics", glm::value_ptr(meshInstance.physicsInfo->scale), 0.1f, 0.0f)) {
                    meshInstance.physicsDebugUBO.scale = meshInstance.physicsInfo->scale;
                }
                if (ImGui::Button("Delete##physics")) {
                    scene.DeletePhysics(meshInstance);
                }
            }
            // Add Script
            ImGui::SeparatorText("Script");
            std::string scriptClassName = Script::GetScriptClassName(meshInstance.UUID);
            if (ImGui::BeginCombo("##ScriptClasses", scriptClassName.c_str())) {
                if (ImGui::MenuItem("None")) {
                    Script::s_scriptInstances.erase(meshInstance.UUID);
                }
                for (auto& scriptClass : Script::s_scriptClasses) {
                    if (ImGui::MenuItem(scriptClass->GetName().c_str())) {
                        Script::s_scriptInstances.emplace(meshInstance.UUID, std::make_shared<ScriptInstance>(scriptClass, meshInstance));
                    }
                }
                ImGui::EndCombo();
            }

            // Add Camera
            ImGui::SeparatorText("Camera");
            if (!meshInstance.camera) {
                if (ImGui::Button("Add##_CAMERA")) {
                    scene.AddCamera(meshInstance);
                    scene.m_selectedCameraUUID = meshInstance.UUID;
                }
            } else {
                if (scene.m_selectedCameraUUID != meshInstance.UUID) {
                    if (ImGui::Button("Select##_CAMERA")) {
                        scene.m_selectedCameraUUID = meshInstance.UUID;
                    }
                } else {
                    if (ImGui::Button("Deselect##_CAMERA")) {
                        scene.m_selectedCameraUUID = 0;
                    }
                }
                if (ImGui::Button("Delete##_CAMERA")) {
                    meshInstance.camera.reset();
                    scene.m_selectedCameraUUID = 0;
                }
            }
        }
        ImGui::EndTabItem();
    }
    // Light List
    if (ImGui::BeginTabItem("Light")) {
        if (ImGui::BeginListBox("##Light", ImVec2(-FLT_MIN, 0.0f))) {
            for (int i = 0; i < scene.m_pointLight.Size(); i++) {
                std::string name(std::string("Point Light ") + std::to_string(i));
                ImGui::PushID(i);
                if (ImGui::Selectable(name.c_str(), i == scene.m_selectedLightIndex)) {
                    scene.UnselectAll();
                    scene.m_selectedLightIndex = i;
                }
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) {
                        scene.m_selectedLightIndex = i;
                        scene.DeletePointLight(commandBuffer);
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
            ImGui::Checkbox("Show Light Icon", &scene.m_showLightIcon);
            // Light Attributes
            if (scene.m_selectedLightIndex > -1) {
                auto& light = scene.m_pointLight.m_UBOs[scene.m_selectedLightIndex];

                ImGui::SeparatorText("Point Light");
                ImGui::DragFloat3("Position##_POINTLIGHT", &light.pos[0], 0.1f);
                if (ImGui::DragFloat("Intensity##_POINTLIGHT", &light.intensity, 0.1f))
                    light.intensity = std::max(0.0f, light.intensity);
                if (ImGui::DragFloat("Range##_POINTLIGHT", &light.range, 0.1f))
                    light.range = std::max(0.0f, light.range);
                ImGui::SliderFloat3("Color##_POINTLIGHT", &light.color[0], 0.0f, 1.0f);
            }
            ImGui::SeparatorText("Shadow Map");
            ImGui::SliderFloat("Cascade Range 1", &scene.m_mainCamera->m_cascadeRanges[0], scene.m_mainCamera->m_zNear, scene.m_mainCamera->m_cascadeRanges[1]);
            ImGui::SliderFloat("Cascade Range 2", &scene.m_mainCamera->m_cascadeRanges[1], scene.m_mainCamera->m_cascadeRanges[0], scene.m_mainCamera->m_cascadeRanges[2]);
            ImGui::SliderFloat("Cascade Range 3", &scene.m_mainCamera->m_cascadeRanges[2], scene.m_mainCamera->m_cascadeRanges[1], scene.m_mainCamera->m_cascadeRanges[3]);
            ImGui::SliderFloat("Cascade Range 4", &scene.m_mainCamera->m_cascadeRanges[3], scene.m_mainCamera->m_cascadeRanges[2], scene.m_mainCamera->m_zFar);
            bool cascadeDebug = scene.m_cascadedShadowMap.m_UBO.debug > 0;
            if (ImGui::Checkbox("Debug", &cascadeDebug))
                scene.m_cascadedShadowMap.m_UBO.debug = cascadeDebug ? 1 : -1;
            ImGui::SeparatorText("Directional Light");
            ImGui::DragFloat3("Position", &scene.m_dirLight.pos[0], 0.1f);
            ImGui::SliderFloat3("Color", &scene.m_dirLight.color[0], 0.0f, 1.0f);
            if (ImGui::DragFloat("Intensity", &scene.m_dirLight.intensity, 0.1f))
                scene.m_dirLight.intensity = std::max(0.0f, scene.m_dirLight.intensity);

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
                scene.AddEnvironmentMap(hdriFilePath, commandBuffer);
            }
        }
        if (!scene.m_hdriFilePath.empty()) {
            ImGui::Text("%s", scene.m_hdriFilePath.c_str());
        }
        if (ImGui::Button("Remove##_IBL")) {
            scene.m_hdriFilePath.clear();
            scene.SelectDummyEnvMap(commandBuffer);
        }
        ImGui::SeparatorText("Exposure");
        ImGui::SliderFloat("Exposure##_IBL", &scene.m_iblExposure, 0.0f, 10.0f);
        if (ImGui::Button("Reset##_IBL")) {
            scene.m_iblExposure = 1.0f;
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void UI::DrawResourceWindow(Scene& scene, const vk::CommandBuffer& commandBuffer)
{
    ImGui::Begin("Resource");
    ImGui::BeginTabBar("Resource");
    if (ImGui::BeginTabItem("Model")) {
        const float resourceButtonSize = 100.0f;
        float panelSize = ImGui::GetContentRegionAvail().x;
        int columnCount = std::max(1, (int)(panelSize / resourceButtonSize));
        // Add Resource
        ImGui::Columns(columnCount, nullptr, false);
        if (ImGui::ImageButton(m_plusIconDescriptorSet, { GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)), GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(resourceButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
            std::string path = nfdOpen({ "Model", "gltf" });
            if (!path.empty()) {
                scene.AddResource(path, commandBuffer);
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
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Script")) {
        const float resourceButtonSize = 100.0f;
        float panelSize = ImGui::GetContentRegionAvail().x;
        int columnCount = std::max(1, (int)(panelSize / resourceButtonSize));
        // Add Resource
        ImGui::Columns(columnCount, nullptr, false);
        if (ImGui::ImageButton(m_plusIconDescriptorSet, { GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)), GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(resourceButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
            // std::string path = nfdOpen({ "Script", "dll" });
            // Script::Reload();
        }
        ImGui::NextColumn();

        // for (int i = 0; i < scene.m_resources.size(); i++) {
        //     ImGui::PushID(i);
        //     ImGui::ImageButton(m_cubeIconDescriptorSet, { GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)), GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(resourceButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
        //     if (ImGui::BeginPopupContextItem()) {
        //         if (ImGui::MenuItem("Delete")) {
        //             // TODO: resource deletion
        //             // scene.m_resources.erase(scene.m_resources.begin() + i);
        //         }
        //         ImGui::EndPopup();
        //     }
        //
        //     // Send Drag Drop
        //     if (ImGui::BeginDragDropSource()) {
        //         ImGui::SetDragDropPayload("RESOURCE_WINDOW_ITEM", (void*)&scene.m_resources[i], sizeof(scene.m_resources[i]));
        //         ImGui::EndDragDropSource();
        //     }
        //
        //     ImGui::PopID();
        //     ImGui::Text("%s", scene.m_resources[i].fileName.c_str());
        //     ImGui::NextColumn();
        // }
        // ImGui::Columns(1);
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void UI::ShowInformationOverlay(const Scene& scene)
{
    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(imguiViewport->Size.x, 0.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("Information", nullptr, window_flags);
    ImGui::Text("%s", std::to_string(Time::GetFrameCount()).c_str());
    ImGui::Text("Camera Control: %s [press C]", scene.m_mainCamera->IsControllable() ? "on" : "off");
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
    scene.AddMeshInstance(*std::static_pointer_cast<Mesh>(s_dragDropResource->ptr.lock()));
    s_dragDropped = false;
}

UI::~UI()
{
    for (auto& layout : m_descriptorSetLayouts)
        vkn::Device::Get().device.destroyDescriptorSetLayout(layout);
    ImGui_ImplVulkan_RemoveTexture(m_viewportImageDescriptorSet);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkn::Device::Get().device.destroyDescriptorPool(m_descriptorPool);
}
