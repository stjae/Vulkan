#include "imgui.h"
#include <font/IconsFontAwesome5.h>

void MyImGui::Setup(const vk::RenderPass& renderPass, Viewport& viewport)
{
    DescriptorSetLayoutData layout;
    layout.descriptorSetCount = 1;
    layout.indices.push_back(0);
    layout.descriptorCounts.push_back(1);
    layout.descriptorTypes.push_back(vk::DescriptorType::eCombinedImageSampler);
    descriptorSetLayoutData_.push_back(layout);

    descriptor_.CreateDescriptorPool(4, descriptorSetLayoutData_, 1, vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(*Window::GetWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Instance::GetHandle().instance;
    init_info.PhysicalDevice = Device::GetHandle().physicalDevice;
    init_info.Device = Device::GetHandle().device;
    init_info.QueueFamily = Queue::GetGraphicsQueueFamilyIndex();
    init_info.Queue = Queue::GetHandle().graphicsQueue;
    init_info.DescriptorPool = descriptor_.descriptorPool_;
    init_info.Subpass = 0;
    init_info.MinImageCount = Swapchain::Get().support.capabilities.minImageCount;
    init_info.ImageCount = Swapchain::Get().frameCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    io.Fonts->AddFontDefault();
    float baseFontSize = 13.0f;                      // 13.0f is the size of the default font. Change to the font size you use.
    float iconFontSize = baseFontSize * 2.0f / 2.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    std::string path(PROJECT_DIR "ttf/");
    path.append(FONT_ICON_FILE_NAME_FAS);
    io.Fonts->AddFontFromFileTTF(path.c_str(), iconFontSize, &icons_config, icons_ranges);
    // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

    // Scale DPI
    if (imguiViewport->DpiScale > 0.0f) {
        ImFontConfig fontConfig;
        fontConfig.SizePixels = 13.0f * imguiViewport->DpiScale;
        io.Fonts->AddFontDefault(&fontConfig);
    }

    viewportDescriptorSets_.resize(Swapchain::Get().frameCount);
    for (int i = 0; i < viewportDescriptorSets_.size(); i++)
        viewportDescriptorSets_[i] = ImGui_ImplVulkan_AddTexture(viewport.frames_[i].viewportImage.GetHandle().sampler,
                                                                 viewport.frames_[i].viewportImage.GetHandle().imageView,
                                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void MyImGui::RecreateViewportDescriptorSets(Viewport& viewport)
{
    for (auto& descriptorSet : viewportDescriptorSets_)
        ImGui_ImplVulkan_RemoveTexture(descriptorSet);

    for (int i = 0; i < viewportDescriptorSets_.size(); i++)
        viewportDescriptorSets_[i] = ImGui_ImplVulkan_AddTexture(viewport.frames_[i].viewportImage.GetHandle().sampler,
                                                                 viewport.frames_[i].viewportImage.GetHandle().imageView,
                                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void MyImGui::DrawImGuizmo(std::unique_ptr<Scene>& scene, int currentItem)
{
    ImGuiIO& io = ImGui::GetIO();

    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene->camera_.isControllable)
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene->camera_.isControllable)
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene->camera_.isControllable)
        OP = ImGuizmo::OPERATION::SCALE;

    // ImGuizmo
    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);

    float translation[3];
    float rotation[3];
    float scale[3];
    float objectMatrix[16];

    auto* modelMat = (glm::mat4*)((uint64_t)scene->uboDataDynamic_.model + (currentItem * scene->uboDataDynamic_.alignment));

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(*modelMat), translation,
                                          rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);
    ImGuizmo::Manipulate(glm::value_ptr(scene->camera_.matrix_.view),
                         glm::value_ptr(scene->camera_.matrix_.proj), OP,
                         ImGuizmo::LOCAL, objectMatrix);
    *modelMat = glm::make_mat4(objectMatrix);
}

void MyImGui::DrawDockSpace(std::unique_ptr<Scene>& scene, Viewport& viewport, size_t frameIndex)
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
            if (ImGui::MenuItem("Open")) {
                std::string filePath = LaunchNfd();
                scene->meshes.emplace_back();
                scene->meshes.back().LoadModel(filePath.c_str(), nullptr);
                scene->meshes.back().CreateBuffers();
                scene->UpdateMesh();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Square")) {
                scene->meshCount_.square++;
                scene->meshes.emplace_back();
                scene->meshes.back().CreateSquare(nullptr, scene->meshCount_.square);
                scene->meshes.back().CreateBuffers();
                scene->UpdateMesh();
            }
            if (ImGui::MenuItem("Cube")) {
                scene->meshCount_.cube++;
                scene->meshes.emplace_back();
                scene->meshes.back().CreateCube(nullptr, scene->meshCount_.cube);
                scene->meshes.back().CreateBuffers();
                scene->UpdateMesh();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    ImGui::End();

    viewport.frames_[frameIndex].command.TransitImageLayout(&viewport.frames_[frameIndex].viewportImage,
                                                            vk::ImageLayout::eUndefined,
                                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                                            {},
                                                            vk::AccessFlagBits::eShaderRead,
                                                            vk::PipelineStageFlagBits::eTopOfPipe,
                                                            vk::PipelineStageFlagBits::eFragmentShader);
    viewport.frames_[frameIndex].command.Submit();

    ImGui::Begin("Viewport");

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    float swapchainRatio = (float)Swapchain::Get().swapchainImageExtent.width / (float)Swapchain::Get().swapchainImageExtent.height;
    float viewportRatio = viewportPanelSize.x / viewportPanelSize.y;
    float t = swapchainRatio / viewportRatio;

    if (viewport.ratio_ != viewportRatio) {
        viewport.ratio_ = viewportRatio;
        if (t <= 1.0f) {
            viewport.extent_.width = (uint32_t)(Swapchain::Get().swapchainImageExtent.width);
            viewport.extent_.height = (uint32_t)((float)Swapchain::Get().swapchainImageExtent.height * t);
        } else {
            viewport.extent_.width = (uint32_t)((float)Swapchain::Get().swapchainImageExtent.width / t);
            viewport.extent_.height = (uint32_t)(Swapchain::Get().swapchainImageExtent.height);
        }

        viewport.RecreateViewportImages();
        RecreateViewportDescriptorSets(viewport);
    }

    ImGui::Image(viewportDescriptorSets_[frameIndex], ImVec2{ viewportPanelSize.x, viewportPanelSize.y });

    ImGui::End();

    viewport.frames_[frameIndex].command.TransitImageLayout(&viewport.frames_[frameIndex].viewportImage,
                                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                                            vk::ImageLayout::eColorAttachmentOptimal,
                                                            vk::AccessFlagBits::eShaderRead,
                                                            vk::AccessFlagBits::eColorAttachmentRead,
                                                            vk::PipelineStageFlagBits::eFragmentShader,
                                                            vk::PipelineStageFlagBits::eColorAttachmentOutput);
    viewport.frames_[frameIndex].command.Submit();
}

void MyImGui::SetCameraControl(std::unique_ptr<Scene>& scene)
{
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        scene->camera_.isControllable = !scene->camera_.isControllable;
        ImGuiIO& io = ImGui::GetIO();

        if (scene->camera_.isControllable) {
            glfwSetInputMode(*Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(*Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        scene->camera_.isInitial = true;
    }
}

void MyImGui::Draw(std::unique_ptr<Scene>& scene, Viewport& viewport, size_t frameIndex)
{
    SetCameraControl(scene);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();

    ImGui::NewFrame();

    DrawDockSpace(scene, viewport, frameIndex);

    static int currentItem = -1;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && !ImGui::IsAnyItemHovered() || currentItem > scene->meshes.size() - 1) // Prevent index out of vector range
        currentItem = -1;

    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && currentItem > -1) {
        scene->meshes.erase(scene->meshes.begin() + currentItem);
        currentItem = -1;
    }

    ImGui::Begin("Object List");
    if (ImGui::BeginListBox("##ObjectList", ImVec2(-FLT_MIN, 0.0f))) {
        for (int i = 0; i < scene->meshes.size(); i++) {
            if (ImGui::Selectable(scene->meshes[i].name.c_str(), scene->meshes[i].isSelected)) {
                currentItem = i;
            }
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("delete")) {
                    scene->meshes.erase(scene->meshes.begin() + i);
                    currentItem = -1;
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndListBox();
    }
    ImGui::End();

    ImGui::Begin("Object Attribute");
    if (currentItem > -1) {
        DrawImGuizmo(scene, currentItem);

        auto* modelMat = (glm::mat4*)((uint64_t)scene->uboDataDynamic_.model + (currentItem * scene->uboDataDynamic_.alignment));

        float translation[3];
        float rotation[3];
        float scale[3];
        float objectMatrix[16];

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(*modelMat), translation,
                                              rotation, scale);

        std::vector<std::string> labels = { "Translation", "Rotation" };
        ImGui::Text("%s", scene->meshes[currentItem].name.c_str());
        ImGui::SliderFloat3(labels[0].append("##translation").c_str(), translation, -10.0f, 10.0f);
        ImGui::SliderFloat3(labels[1].append("##rotation").c_str(), rotation, -180.0f, 180.0f);

        ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);

        *modelMat = glm::make_mat4(objectMatrix);
    }
    ImGui::End();

    ImGui::Begin("Resources");
    ImGui::BeginChild("##", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_ChildWindow);
    if (ImGui::Button(ICON_FA_PLUS, { 100, 100 })) {
        std::string path = LaunchNfd();
        scene->resources_.push_back({ "model", path });
    }
    ImGui::EndChild();

    // Drag Drop Source
    for (auto& resource : scene->resources_) {
        ImGui::Button(ICON_FA_CUBE, { 100, 100 });

        if (ImGui::BeginDragDropSource()) {
            ImGui::Text("%s", resource.at(1).c_str());
            ImGui::SetDragDropPayload("RESOURCE_WINDOW_ITEM", (void*)resource.at(1).c_str(), sizeof(char) * resource.at(1).length());
            ImGui::EndDragDropSource();
        }

        ImGui::TextWrapped("%s", resource.at(0).c_str());
    }
    ImGui::End();

    // Information Overlay
    ImGui::SetNextWindowPos(ImVec2(imguiViewport->Size.x, 0.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("Information", nullptr, window_flags);
    ImGui::Text("%s", GetFrameRate().c_str());
    ImGui::Text("Camera Control: %s [press C]", scene->camera_.isControllable ? "on" : "off");
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
}

MyImGui::~MyImGui()
{
    for (auto& descriptorSet : viewportDescriptorSets_)
        ImGui_ImplVulkan_RemoveTexture(descriptorSet);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}