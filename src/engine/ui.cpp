#include "ui.h"
#include "font/IconsFontAwesome5.h"
#include <filesystem>
#include "../../imgui/imgui_stdlib.h"

void UI::Init(const vk::RenderPass& renderPass, Viewport& viewport, Scene& scene)
{
    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffer);
    vkn::Command::Begin(m_commandBuffer);

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

    ImGui_ImplGlfw_InitForVulkan(Window::GetWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vkn::Instance::GetInstance();
    init_info.PhysicalDevice = vkn::Device::Get().physicalDevice;
    init_info.Device = vkn::Device::Get().device;
    init_info.QueueFamily = vkn::Device::Get().graphicsFamilyIndex.value();
    init_info.Queue = vkn::Device::Get().graphicsQueue;
    init_info.DescriptorPool = m_descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
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

    m_plusIcon.InsertImage(PROJECT_DIR "image/icon/plus.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_plusIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_plusIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_lightIcon.InsertImage(PROJECT_DIR "image/icon/lightbulb.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_lightIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_lightIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_cameraIcon.InsertImage(PROJECT_DIR "image/icon/camera.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_cameraIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_cameraIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_noCameraIcon.InsertImage(PROJECT_DIR "image/icon/no_camera.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_noCameraIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_noCameraIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_cubeIcon.InsertImage(PROJECT_DIR "image/icon/cube.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_cubeIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_cubeIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_playIcon.InsertImage(PROJECT_DIR "image/icon/play.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_playIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_playIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    m_stopIcon.InsertImage(PROJECT_DIR "image/icon/stop.png", vk::Format::eR8G8B8A8Srgb, m_commandBuffer);
    m_stopIconDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, m_stopIcon.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    s_dragDropped = false;

    vkn::Command::End(m_commandBuffer);
    vkn::Command::SubmitAndWait(m_commandBuffer);
}

void UI::Draw(Scene& scene, Viewport& viewport, bool& init)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::NewFrame();

    if ((ImGui::IsKeyPressed(ImGuiKey_Q) && !scene.m_mainCamera.IsControllable()) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        scene.UnselectAll();
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && scene.m_selectedMeshID > -1 && scene.m_selectedMeshInstanceID > -1) {
        scene.DeleteMeshInstance(scene.GetSelectedMesh(), scene.GetSelectedMeshInstance());
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && scene.m_selectedLightIndex > -1) {
        scene.DeletePointLight();
    }

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, scene.IsPlaying());
    DrawDockSpace(scene, init);
    DrawViewport(scene, viewport);
    DrawSceneAttribWindow(scene);
    DrawResourceWindow(scene);
    ShowInformationOverlay();
    ImGui::PopItemFlag();

    ImGui::EndFrame();
    ImGui::Render();
}

void UI::DrawInitPopup(bool& init, Scene& scene)
{
    ImGui::OpenPopup("Welcome");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Welcome", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::Button("New Scene", ImVec2(120, 0))) {
            std::string sceneFilePath = nfdSave({ "Scene", "scn" });
            if (!sceneFilePath.empty()) {
                scene.m_sceneFolderPath = sceneFilePath.substr(0, sceneFilePath.rfind('.'));
                std::filesystem::create_directory(scene.m_sceneFolderPath);
                scene.m_sceneFilePath = scene.m_sceneFolderPath + sceneFilePath.substr(sceneFilePath.find_last_of("/\\"), sceneFilePath.length() - 1);
                SceneSerializer serializer;
                serializer.Serialize(scene);
                init = false;
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::Button("Open Scene", ImVec2(120, 0))) {
            std::string openFilePath = nfdOpen({ "Scene", "scn" });
            if (!openFilePath.empty()) {
                SceneSerializer serializer;
                serializer.Deserialize(scene, openFilePath);
                init = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

void UI::DrawDockSpace(Scene& scene, bool& init)
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
        bool openNewScenePopup = false;
        bool openSaveAsPopup = false;
        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("New")) {
                openNewScenePopup = true;
            }
            if (ImGui::MenuItem("Open")) {
                std::string sceneFilePath = nfdOpen({ "Scene", "scn" });
                if (!sceneFilePath.empty()) {
                    scene.m_sceneFilePath = sceneFilePath;
                    SceneSerializer serializer;
                    serializer.Deserialize(scene, sceneFilePath);
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
                openSaveAsPopup = true;
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
        if (openNewScenePopup)
            ImGui::OpenPopup("New Scene");

        // open save as popup modal
        if (openSaveAsPopup)
            ImGui::OpenPopup("Save As");

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("New Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure to open a new scene?\nUnsaved work will be lost.");
            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PopStyleVar();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                std::string sceneFilePath = nfdSave({ "Scene", "scn" });
                if (!sceneFilePath.empty()) {
                    scene.Clear();
                    scene.m_sceneFolderPath = sceneFilePath.substr(0, sceneFilePath.rfind('.'));
                    std::filesystem::create_directory(scene.m_sceneFolderPath);
                    scene.m_sceneFilePath = scene.m_sceneFolderPath + sceneFilePath.substr(sceneFilePath.find_last_of("/\\"), sceneFilePath.length() - 1);
                    scene.m_mainCamera.SetInitPos();
                    SceneSerializer serializer;
                    serializer.Serialize(scene);
                    scene.SelectDummyEnvMap();
                }
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopupModal("Save As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static std::string sceneName;
            static std::string message;
            ImGui::PushItemWidth(248);
            ImGui::InputTextWithHint("##SceneName", "Scene Name", &sceneName);
            ImGui::PopItemWidth();
            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PopStyleVar();

            if (!message.empty())
                ImGui::TextWrapped("%s", message.c_str());
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                if (!sceneName.empty()) {
                    scene.m_sceneFilePath = scene.m_sceneFolderPath + "\\" + sceneName + ".scn";
                    if (!std::filesystem::exists(scene.m_sceneFilePath)) {
                        SceneSerializer serializer;
                        serializer.Serialize(scene);
                        sceneName = "";
                        message = "";
                        ImGui::CloseCurrentPopup();
                    } else {
                        message = "Scene file with the same name exists";
                    }
                } else {
                    message = "Please insert name";
                }
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                sceneName = "";
                message = "";
            }
            ImGui::EndPopup();
        }
        if (init) {
            DrawInitPopup(init, scene);
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
    float currentRatio = viewport.m_panelSize.x / viewport.m_panelSize.y;

    if ((Viewport::s_panelRatio != currentRatio) || viewport.m_outDated) {
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

    DrawLightIcon(scene, viewport);
    DrawCameraIcon(scene, viewport);

    if (scene.m_selectedMeshID > -1) {
        DrawMeshGuizmo(scene, viewport);
    }

    if (scene.m_selectedLightIndex > -1) {
        DrawLightGuizmo(scene, viewport);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void UI::DrawMeshGuizmo(Scene& scene, const Viewport& viewport)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.m_mainCamera.IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.m_mainCamera.IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.m_mainCamera.IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewport.m_panelPos.x, viewport.m_panelPos.y, viewport.m_panelSize.x, viewport.m_panelSize.y);

    auto& mesh = scene.GetSelectedMesh();
    auto& meshInstance = scene.GetSelectedMeshInstance();

    if (ImGuizmo::Manipulate(glm::value_ptr(scene.m_mainCamera.GetUBO().view), glm::value_ptr(scene.m_mainCamera.GetUBO().proj), OP, ImGuizmo::LOCAL, glm::value_ptr(meshInstance.UBO.model))) {
        meshInstance.UpdateTransform();
        mesh.UpdateUBO(meshInstance);
    }
}

void UI::DrawLightGuizmo(Scene& scene, const Viewport& viewport)
{
    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene.m_mainCamera.IsControllable())
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene.m_mainCamera.IsControllable())
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene.m_mainCamera.IsControllable())
        OP = ImGuizmo::OPERATION::SCALE;

    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewport.m_panelPos.x, viewport.m_panelPos.y, viewport.m_panelSize.x, viewport.m_panelSize.y);

    auto& light = scene.m_pointLight.m_UBOs[scene.m_selectedLightIndex];
    glm::mat4 lightTranslation = glm::translate(glm::mat4(1.0f), light.pos);
    ImGuizmo::Manipulate(glm::value_ptr(scene.m_mainCamera.GetUBO().view), glm::value_ptr(scene.m_mainCamera.GetUBO().proj), OP, ImGuizmo::LOCAL, glm::value_ptr(lightTranslation));
    light.pos = lightTranslation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void UI::DrawSceneAttribWindow(Scene& scene)
{
    ImGui::Begin("Scene");
    ImGui::BeginTabBar("Scene");
    // Mesh List
    if (ImGui::BeginTabItem("Mesh")) {
        bool openCreateInstancePopup = false;
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
                            if (ImGui::MenuItem("Create Instance")) {
                                openCreateInstancePopup = true;
                            }
                            ImGui::EndPopup();
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
        if (openCreateInstancePopup)
            ImGui::OpenPopup("Create Instance");
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Create Instance", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static float offset[3] = { 1.0f, 1.0f, 1.0f };
            static int amount[3] = { 1, 1, 1 };
            ImGui::PushItemWidth(100);
            ImGui::InputInt("Amount X", &amount[0]);
            ImGui::SameLine();
            ImGui::InputFloat("Offset X", &offset[0]);
            ImGui::InputInt("Amount Y", &amount[1]);
            ImGui::SameLine();
            ImGui::InputFloat("Offset Y", &offset[1]);
            ImGui::InputInt("Amount Z", &amount[2]);
            ImGui::SameLine();
            ImGui::InputFloat("Offset Z", &offset[2]);
            ImGui::Separator();
            if (ImGui::Button("Create", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
                for (int z = 0; z < amount[2]; z++) {
                    for (int y = 0; y < amount[1]; y++) {
                        for (int x = 0; x < amount[0]; x++) {
                            if (x == 0 && y == 0 && z == 0)
                                continue;
                            scene.DuplicateMeshInstance(scene.m_selectedMeshID, scene.m_selectedMeshInstanceID, { offset[0] * x, offset[1] * y, offset[2] * z });
                        }
                    }
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        // Mesh Attributes
        if (scene.m_selectedMeshID > -1 && scene.m_selectedMeshInstanceID > -1) {
            auto& mesh = scene.GetSelectedMesh();
            auto& meshInstance = scene.GetSelectedMeshInstance();

            ImGui::SeparatorText((std::string("ID: ") + std::to_string(meshInstance.UUID)).c_str());
            ImGui::Separator();
            if (ImGui::DragFloat3("Translation##_INSTANCE", &meshInstance.translation[0], 0.1f)) {
                meshInstance.UpdateMatrix();
                mesh.UpdateUBO(meshInstance);
            }
            if (ImGui::DragFloat3("Rotation##_INSTANCE", &meshInstance.rotation[0], 0.1f)) {
                meshInstance.UpdateMatrix();
                mesh.UpdateUBO(meshInstance);
            }
            if (ImGui::DragFloat3("Scale##_INSTANCE", &meshInstance.scale[0], 0.1f)) {
                meshInstance.UpdateMatrix();
                mesh.UpdateUBO(meshInstance);
            }

            ImGui::SeparatorText("Texture");
            bool useAlbedoTexture = meshInstance.UBO.useAlbedoTexture > 0;
            if (ImGui::Checkbox("Use Albedo Texture", &useAlbedoTexture)) {
                meshInstance.UBO.useAlbedoTexture = useAlbedoTexture ? 1 : -1;
                mesh.UpdateUBO(meshInstance);
            }
            bool useNormalTexture = meshInstance.UBO.useNormalTexture > 0;
            if (ImGui::Checkbox("Use Normal Texture", &useNormalTexture)) {
                meshInstance.UBO.useNormalTexture = useNormalTexture ? 1 : -1;
                mesh.UpdateUBO(meshInstance);
            }
            bool useMetallicTexture = meshInstance.UBO.useMetallicTexture > 0;
            if (ImGui::Checkbox("Use Metallic Texture", &useMetallicTexture)) {
                meshInstance.UBO.useMetallicTexture = useMetallicTexture ? 1 : -1;
                mesh.UpdateUBO(meshInstance);
            }
            bool useRoughnessTexture = meshInstance.UBO.useRoughnessTexture > 0;
            if (ImGui::Checkbox("Use Roughness Texture", &useRoughnessTexture)) {
                meshInstance.UBO.useRoughnessTexture = useRoughnessTexture ? 1 : -1;
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
                    scene.AddPhysics(*scene.m_meshes[scene.m_selectedMeshID], meshInstance, physicsInfo);
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
            if (ImGui::Button("Load"))
                Script::LoadDll(scene.m_sceneFolderPath);
            std::string scriptClassName = Script::GetScriptClassName(meshInstance.UUID);
            if (ImGui::BeginCombo("##ScriptClasses", scriptClassName.c_str())) {
                if (ImGui::MenuItem("None")) {
                    Script::s_scriptInstances.erase(meshInstance.UUID);
                }
                for (auto& scriptClass : Script::s_scriptClasses) {
                    if (ImGui::MenuItem(scriptClass.first.c_str())) {
                        if (Script::InstanceExists(meshInstance.UUID))
                            Script::s_scriptInstances.erase(meshInstance.UUID);
                        Script::s_scriptInstances.emplace(meshInstance.UUID, std::make_shared<ScriptInstance>(scriptClass.second, meshInstance.UUID));
                    }
                }
                ImGui::EndCombo();
            }

            // Add Camera
            ImGui::SeparatorText("Camera");
            if (!meshInstance.camera) {
                if (ImGui::Button("Add##_CAMERA")) {
                    scene.AddCamera(meshInstance);
                }
            } else {
                if (scene.m_playCamera->GetID() != meshInstance.UUID) {
                    if (ImGui::Button("Select##_CAMERA")) {
                        scene.m_playCamera = meshInstance.camera.get();
                    }
                } else {
                    if (ImGui::Button("Unselect##_CAMERA")) {
                        scene.m_playCamera = &scene.m_mainCamera;
                    }
                }
                if (ImGui::Button("Delete##_CAMERA")) {
                    scene.DeleteCamera(meshInstance.UUID);
                    meshInstance.camera.reset();
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
                        scene.DeletePointLight();
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
            if (ImGui::Button("Add")) {
                scene.AddPointLight();
            }
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
            ImGui::SeparatorText("Directional Light");
            ImGui::DragFloat3("Position", &scene.m_dirLight.pos[0], 0.1f);
            ImGui::SliderFloat3("Color", &scene.m_dirLight.color[0], 0.0f, 1.0f);
            if (ImGui::DragFloat("Intensity", &scene.m_dirLight.intensity, 0.1f))
                scene.m_dirLight.intensity = std::max(0.0f, scene.m_dirLight.intensity);
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
        if (ImGui::Button("Remove##_IBL")) {
            scene.m_hdriFilePath.clear();
            scene.SelectDummyEnvMap();
        }
        ImGui::SeparatorText("Exposure");
        ImGui::SliderFloat("Exposure##_IBL", &scene.m_iblExposure, 0.0f, 10.0f);
        if (ImGui::Button("Reset##_IBL")) {
            scene.m_iblExposure = 1.0f;
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Camera")) {
        ImGui::SeparatorText("Play Camera");
        if (scene.m_playCamera->GetID() == 0) {
            ImGui::Text("Main Camera");
        } else {
            ImGui::Text("ID:%s", std::to_string(scene.m_playCamera->GetID()).c_str());
        }
        ImGui::SeparatorText("Cascade");
        ImGui::SliderFloat("Cascade Range 1", &Camera::s_cascadeRanges[0], Camera::s_zNear, Camera::s_cascadeRanges[1]);
        ImGui::SliderFloat("Cascade Range 2", &Camera::s_cascadeRanges[1], Camera::s_cascadeRanges[0], Camera::s_cascadeRanges[2]);
        ImGui::SliderFloat("Cascade Range 3", &Camera::s_cascadeRanges[2], Camera::s_cascadeRanges[1], Camera::s_cascadeRanges[3]);
        ImGui::SliderFloat("Cascade Range 4", &Camera::s_cascadeRanges[3], Camera::s_cascadeRanges[2], Camera::s_zFar);
        bool cascadeDebug = scene.m_cascadedShadowMap.m_UBO.debug > 0;
        if (ImGui::Checkbox("Show Cascade Division", &cascadeDebug))
            scene.m_cascadedShadowMap.m_UBO.debug = cascadeDebug ? 1 : -1;
        ImGui::SeparatorText("Post Effect");
        bool useMotionBlur = postProcessPushConstants.useMotionBlur > 0;
        if (ImGui::Checkbox("Motion Blur", &useMotionBlur)) {
            postProcessPushConstants.useMotionBlur = useMotionBlur ? 1 : -1;
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("etc")) {
        if (ImGui::SliderInt("Grid Size", &scene.m_gridWidth, 2, 100)) {
            if (scene.m_gridWidth % 2 != 0)
                scene.m_gridWidth++;
            scene.CreateGrid();
        }
        ImGui::Checkbox("Show Grid", &scene.m_showGrid);
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void UI::DrawResourceWindow(Scene& scene)
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
                scene.AddResource(path);
            }
        }
        ImGui::NextColumn();

        int meshIndexToDelete = -1;
        for (int i = 0; i < scene.m_meshes.size(); i++) {
            ImGui::PushID(i);
            ImGui::ImageButton(m_cubeIconDescriptorSet, { GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)), GetIconSize(resourceButtonSize, GetButtonPadding(resourceButtonSize, 0.4f)) }, ImVec2(0, 0), ImVec2(1, 1), GetButtonPadding(resourceButtonSize, 0.4f), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete")) {
                    meshIndexToDelete = i;
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
        if (meshIndexToDelete > -1) {
            scene.DeleteMesh(meshIndexToDelete);
            scene.m_resources.erase(scene.m_resources.begin() + meshIndexToDelete);
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}

void UI::DrawLightIcon(const Scene& scene, const Viewport& viewport)
{
    for (auto& light : scene.m_pointLight.m_UBOs) {
        glm::vec4 pos = scene.m_mainCamera.GetUBO().proj * scene.m_mainCamera.GetUBO().view * glm::vec4(light.pos, 1.0f);
        float posZ = pos.z;
        pos /= pos.w;
        pos.x = (pos.x + 1.0f) * 0.5f;
        pos.y = 1.0f - (pos.y + 1.0f) * 0.5f;
        pos.x *= (float)viewport.m_panelSize.x;
        pos.y *= (float)viewport.m_panelSize.y;
        ImVec2 screenPos(pos.x, pos.y);
        ImVec2 offset(300, 300);
        offset /= posZ;
        if (posZ > 1.0f && scene.m_showLightIcon && !scene.IsPlaying())
            ImGui::GetWindowDrawList()->AddImage(m_lightIconDescriptorSet, viewport.m_panelPos + screenPos - offset, viewport.m_panelPos + screenPos + offset, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_BLACK);
    }
}

void UI::DrawCameraIcon(const Scene& scene, const Viewport& viewport)
{
    for (auto& camera : scene.m_cameras) {
        glm::vec4 pos = scene.m_mainCamera.GetUBO().proj * scene.m_mainCamera.GetUBO().view * glm::vec4(camera.second->m_pos, 1.0f);
        float posZ = pos.z;
        pos /= pos.w;
        pos.x = (pos.x + 1.0f) * 0.5f;
        pos.y = 1.0f - (pos.y + 1.0f) * 0.5f;
        pos.x *= (float)viewport.m_panelSize.x;
        pos.y *= (float)viewport.m_panelSize.y;
        ImVec2 screenPos(pos.x, pos.y);
        ImVec2 offset(300, 300);
        offset /= posZ;
        if (posZ > 1.0f && scene.m_showLightIcon && !scene.IsPlaying()) {
            if (scene.m_playCamera == camera.second)
                ImGui::GetWindowDrawList()->AddImage(m_cameraIconDescriptorSet, viewport.m_panelPos + screenPos - offset, viewport.m_panelPos + screenPos + offset, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_BLACK);
            else
                ImGui::GetWindowDrawList()->AddImage(m_noCameraIconDescriptorSet, viewport.m_panelPos + screenPos - offset, viewport.m_panelPos + screenPos + offset, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_BLACK);
        }
    }
}

void UI::ShowInformationOverlay()
{
    ImGui::Begin("Information");
    ImGui::Text("FPS: %s", std::to_string(Time::GetFrameCount()).c_str());
    ImGui::TextWrapped("Instruction: press W S A D Q E to move camera, press C to turn camera control on / off");
    ImGui::End();
}

void UI::RecreateViewportDescriptorSet(const Viewport& viewport)
{
    ImGui_ImplVulkan_RemoveTexture(m_viewportImageDescriptorSet);
    m_viewportImageDescriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::s_repeatSampler, viewport.m_imageFinal.Get().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void UI::AcceptDragDrop(Scene& scene)
{
    if (!s_dragDropped)
        return;

    scene.AddMeshInstance(*(Mesh*)s_dragDropResource->ptr);
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
