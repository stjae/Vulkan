#include "ui.h"
#include "font/IconsFontAwesome5.h"

void UI::Setup(const vk::RenderPass& renderPass, Viewport& viewport)
{
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

    descriptorSets_.resize(Swapchain::GetBundle().frameImageCount);
    for (int i = 0; i < descriptorSets_.size(); i++)
        descriptorSets_[i] = ImGui_ImplVulkan_AddTexture(viewport.frames[i].viewportImage.GetBundle().sampler,
                                                         viewport.frames[i].viewportImage.GetBundle().imageView,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
        scene.selectedInstanceID = -1;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        scene.DeleteMesh();
    }

    DrawDockSpace(scene);
    DrawViewport(scene, viewport, frameIndex);
    DrawObjectWindow(scene);
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
            // if (ImGui::MenuItem("Open")) {
            //     std::string filePath = LaunchNfd({ "model", "obj" });
            //     scene.AddMesh();
            // }
            // ImGui::Separator();
            if (ImGui::MenuItem("Square")) {
                scene.AddMesh(MESHTYPE::SQUARE);
            }
            if (ImGui::MenuItem("Cube")) {
                scene.AddMesh(MESHTYPE::CUBE);
            }
            if (ImGui::MenuItem("Sphere")) {
                scene.AddMesh(MESHTYPE::SPHERE);
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

    auto& frame = viewport.frames[frameIndex];

    Command::Begin(frame.commandBuffer);
    Command::SetImageMemoryBarrier(frame.commandBuffer, frame.viewportImage,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   {},
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    Command::SetImageMemoryBarrier(frame.commandBuffer, frame.colorID,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   {},
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    frame.commandBuffer.end();
    Command::Submit(&frame.commandBuffer, 1);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    viewport.panelPos = ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMin();
    viewport.panelSize = ImGui::GetContentRegionAvail();
    // debug: drawRect
    //    ImGui::GetForegroundDrawList()->AddRect(viewport.panelPos, viewport.panelPos + viewport.panelSize, IM_COL32(255, 0, 0, 255));
    float viewportPanelRatio = viewport.panelSize.x / viewport.panelSize.y;

    if (viewport.panelRatio != viewportPanelRatio || viewport.outDated)
        SetViewportUpToDate(viewport, viewport.panelSize);

    // Accept Drag Drop
    ImGui::Image(descriptorSets_[frameIndex], ImVec2{ viewport.panelSize.x, viewport.panelSize.y });
    viewport.isMouseHovered = ImGui::IsItemHovered();
    if (ImGui::BeginDragDropTarget()) {

        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_WINDOW_ITEM", ImGuiDragDropFlags_AcceptBeforeDelivery);
        Resource* data = nullptr;
        if (payload)
            data = (Resource*)payload->Data;

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && data) {

            // TODO: value is always 0 on Windows OS
            const int32_t* pickColor = viewport.PickColor(frameIndex);

            switch (data->resourceType) {
            case RESOURCETYPE::MESH:
                scene.AddMesh(static_cast<Mesh*>(data->resource)->meshID);
                break;
            case RESOURCETYPE::TEXTURE:
                if (pickColor[0] != -1) {
                    auto& instanceData = scene.meshes[pickColor[0]].instanceData_[pickColor[1]];

                    instanceData.textureID = static_cast<Texture*>(data->resource)->index;
                    instanceData.useTexture = true;
                }
                break;
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (scene.selectedMeshID != -1) {
        DrawImGuizmo(scene, viewport.panelPos);
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

    viewport.DestroyViewportImages();
    viewport.CreateViewportImages();
    RecreateViewportDescriptorSets(viewport);

    viewport.outDated = false;
}

void UI::DrawImGuizmo(Scene& scene, const ImVec2& viewportPanelPos)
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
    float objectMatrix[16];

    auto& meshInstanceData = scene.meshes[scene.selectedMeshID].instanceData_[scene.selectedInstanceID];
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(meshInstanceData.model), translation,
                                          rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);
    ImGuizmo::Manipulate(glm::value_ptr(scene.camera.GetMatrix().view),
                         glm::value_ptr(scene.camera.GetMatrix().proj), OP,
                         ImGuizmo::LOCAL, objectMatrix);
    meshInstanceData.model = glm::make_mat4(objectMatrix);
    meshInstanceData.invTranspose = glm::make_mat4(objectMatrix);
    meshInstanceData.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    meshInstanceData.invTranspose = glm::transpose(glm::inverse(meshInstanceData.invTranspose));
}

void UI::DrawObjectWindow(Scene& scene)
{
    // Mesh List
    ImGui::Begin("Mesh List");
    if (ImGui::BeginListBox("##MeshList", ImVec2(-FLT_MIN, 0.0f))) {
        for (int i = 0; i < scene.meshes.size(); i++) {
            std::string name(scene.meshes[i].GetName());
            for (int j = 0; j < scene.meshes[i].instanceData_.size(); j++) {
                ImGui::PushID(i * j + j);
                if (ImGui::Selectable(name.c_str(), i == scene.selectedMeshID && j == scene.selectedInstanceID)) {
                    scene.selectedMeshID = i;
                    scene.selectedInstanceID = j;
                }
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("delete")) {
                        scene.selectedMeshID = i;
                        scene.selectedInstanceID = j;
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

        auto& meshInstanceData = scene.meshes[scene.selectedMeshID].instanceData_[scene.selectedInstanceID];

        float translation[3];
        float rotation[3];
        float scale[3];
        float objectMatrix[16];

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(meshInstanceData.model), translation,
                                              rotation, scale);

        std::vector<std::string> labels = { "Move", "Rotate" };
        ImGui::SeparatorText("Translation");
        ImGui::SliderFloat3(labels[0].append("##translation").c_str(), translation, -10.0f, 10.0f);
        ImGui::SliderFloat3(labels[1].append("##rotation").c_str(), rotation, -180.0f, 180.0f);

        ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);

        meshInstanceData.model = glm::make_mat4(objectMatrix);
        meshInstanceData.invTranspose = glm::make_mat4(objectMatrix);
        meshInstanceData.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        meshInstanceData.invTranspose = glm::transpose(glm::inverse(meshInstanceData.invTranspose));

        ImGui::SeparatorText("Textures");
        bool useTexture = meshInstanceData.useTexture;
        ImGui::Checkbox("Use Texture", &useTexture);
        meshInstanceData.useTexture = useTexture ? 1 : 0;
    }
    ImGui::End();
}

void UI::DrawResourceWindow(Scene& scene)
{
    ImGui::Begin("Resources");
    // Add Resource
    if (ImGui::Button(ICON_FA_PLUS, { 100, 100 })) {
        std::string path = LaunchNfd({ "Image,Mesh", "jpg,png,obj" });
        if (!path.empty()) {
            scene.AddResource(path);
        }
    }

    // Send Drag Drop
    for (auto& resource : scene.resources) {
        ImGui::PushID(resource.filePath.c_str());
        if (resource.resourceType == RESOURCETYPE::MESH)
            ImGui::Button(ICON_FA_CUBE, { 100, 100 });
        else {
            ImGui::ImageButton(static_cast<Texture*>(resource.resource)->descriptorSet, { 100, 100 }, { 0, 0 }, { 1, 1 }, 0);
        }

        if (ImGui::BeginDragDropSource()) {
            if (resource.resourceType == RESOURCETYPE::TEXTURE)
                ImGui::Image(static_cast<Texture*>(resource.resource)->descriptorSet, { 100, 100 }, { 0, 0 }, { 1, 1 });
            ImGui::SetDragDropPayload("RESOURCE_WINDOW_ITEM", (void*)&resource, sizeof(resource));
            ImGui::EndDragDropSource();
        }

        ImGui::TextWrapped("%s", resource.fileName.c_str());
        ImGui::PopID();
    }
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
    for (auto& descriptorSet : descriptorSets_)
        ImGui_ImplVulkan_RemoveTexture(descriptorSet);

    for (int i = 0; i < descriptorSets_.size(); i++)
        descriptorSets_[i] = ImGui_ImplVulkan_AddTexture(viewport.frames[i].viewportImage.GetBundle().sampler,
                                                         viewport.frames[i].viewportImage.GetBundle().imageView,
                                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

UI::~UI()
{
    for (auto& layout : descriptorSetLayouts_)
        Device::GetBundle().device.destroyDescriptorSetLayout(layout);
    for (auto& set : descriptorSets_)
        ImGui_ImplVulkan_RemoveTexture(set);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Device::GetBundle().device.destroyDescriptorPool(descriptorPool_);
}
