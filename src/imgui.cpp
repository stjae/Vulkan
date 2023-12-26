#include "imgui.h"

void MyImGui::Setup(GraphicsPipeline& pipeline)
{
    {
        vk::DescriptorPoolSize poolSizes[] = {
            { vk::DescriptorType::eCombinedImageSampler, 1 },
        };

        vk::DescriptorPoolCreateInfo poolInfo(
            vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1,
            static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes)), poolSizes);

        if (Device::GetHandle().device.createDescriptorPool(
                &poolInfo, nullptr, &imGuiDescriptorPool) != vk::Result::eSuccess) {
            spdlog::error("failed to create descriptor pool for ImGui");
        }
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(*Window::GetWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Instance::GetHandle().instance;
    init_info.PhysicalDevice = Device::GetHandle().physicalDevice;
    init_info.Device = Device::GetHandle().device;
    init_info.QueueFamily = Queue::GetGraphicsQueueFamilyIndex();
    init_info.Queue = Queue::GetHandle().graphicsQueue;
    init_info.DescriptorPool = imGuiDescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = Swapchain::GetSupportDetail().capabilities.minImageCount;
    init_info.ImageCount = Swapchain::GetDetail().frames.size();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, pipeline.GetHandle().renderPass);

    if (viewport->DpiScale > 0.0f) {
        ImFontConfig fontConfig;
        fontConfig.SizePixels = 13.0f * viewport->DpiScale;
        io.Fonts->AddFontDefault(&fontConfig);
    }
}

void MyImGui::DrawImGuizmo(std::unique_ptr<Scene>& scene, int currentItem)
{
    ImGuiIO& io = ImGui::GetIO();

    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene->camera_->isControllable)
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene->camera_->isControllable)
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene->camera_->isControllable)
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
    ImGuizmo::Manipulate(glm::value_ptr(scene->camera_->matrix_.view),
                         glm::value_ptr(scene->camera_->matrix_.proj), OP,
                         ImGuizmo::LOCAL, objectMatrix);
    *modelMat = glm::make_mat4(objectMatrix);
}

void MyImGui::DrawDockSpace(std::unique_ptr<Scene>& scene)
{
    ImGuiIO& io = ImGui::GetIO();

    static bool p_open = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;
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
                scene->meshes.emplace_back(std::make_shared<Mesh>());
                scene->meshes.back()->LoadModel(filePath.c_str(), nullptr);
                scene->meshes.back()->CreateBuffers();
                scene->UpdateMesh();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Square")) {
                scene->meshCount_.square++;
                scene->meshes.emplace_back(std::make_shared<Mesh>());
                scene->meshes.back()->CreateSquare(nullptr, scene->meshCount_.square);
                scene->meshes.back()->CreateBuffers();
                scene->UpdateMesh();
            }
            if (ImGui::MenuItem("Cube")) {
                scene->meshCount_.cube++;
                scene->meshes.emplace_back(std::make_shared<Mesh>());
                scene->meshes.back()->CreateCube(nullptr, scene->meshCount_.cube);
                scene->meshes.back()->CreateBuffers();
                scene->UpdateMesh();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    ImGui::End();
}

void MyImGui::SetCameraControl(std::unique_ptr<Scene>& scene)
{
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        scene->camera_->isControllable = !scene->camera_->isControllable;
        ImGuiIO& io = ImGui::GetIO();

        if (scene->camera_->isControllable) {
            glfwSetInputMode(*Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(*Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        scene->camera_->isInitial = true;
    }
}

void MyImGui::Draw(std::unique_ptr<Scene>& scene)
{
    SetCameraControl(scene);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::NewFrame();

    DrawDockSpace(scene);

    if (scene) {

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
                if (ImGui::Selectable(scene->meshes[i]->name.c_str(), scene->meshes[i]->isSelected)) {
                    currentItem = i;
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

            std::string id = "##";
            id.append(scene->meshes[currentItem]->name);
            std::vector<std::string> labels = { "Translation", "Rotation" };
            ImGui::Text("%s", scene->meshes[currentItem]->name.c_str());
            ImGui::SliderFloat3(labels[0].append(id).c_str(), translation, -10.0f, 10.0f);
            ImGui::SliderFloat3(labels[1].append(id).c_str(), rotation, -180.0f, 180.0f);

            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale,
                                                    objectMatrix);

            *modelMat = glm::make_mat4(objectMatrix);
        }
        ImGui::End();

        ImGui::Begin("Resource Library");
        ImGui::End();
    }

    // Information Overlay
    ImGui::SetNextWindowPos(ImVec2(viewport->Size.x, 0.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("Information", nullptr, window_flags);
    ImGui::Text("%s", GetFrameRate().c_str());
    ImGui::Text("Camera Control: %s [press C]", scene->camera_->isControllable ? "on" : "off");
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
}

MyImGui::~MyImGui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    Device::GetHandle().device.destroyDescriptorPool(imGuiDescriptorPool);
    ImGui::DestroyContext();
}