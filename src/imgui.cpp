#include "imgui.h"

void MyImGui::Setup(const std::unique_ptr<GraphicsEngine>& engine)
{
    {
        vk::DescriptorPoolSize poolSizes[] = {
            { vk::DescriptorType::eCombinedImageSampler, 1 },
        };

        vk::DescriptorPoolCreateInfo poolInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes)), poolSizes);

        if (engine->device.vkDevice.createDescriptorPool(&poolInfo, nullptr, &engine->device.imGuiDescriptorPool) != vk::Result::eSuccess) {
            spdlog::error("failed to create descriptor pool for ImGui");
        }
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(engine->window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = engine->device.instance.vkInstance;
    init_info.PhysicalDevice = engine->device.vkPhysicalDevice;
    init_info.Device = engine->device.vkDevice;
    init_info.QueueFamily = engine->device.queueFamilyIndices.graphicsFamily.value();
    init_info.Queue = engine->device.vkGraphicsQueue;
    init_info.DescriptorPool = engine->device.imGuiDescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = engine->swapchain.supportDetail.capabilities.minImageCount;
    init_info.ImageCount = engine->maxFrameNumber;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, engine->pipeline.vkRenderPass);

#if defined(_WIN32)
    float dpiScaleFactor = ImGui_ImplWin32_GetDpiScaleForHwnd(GetDesktopWindow());

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 13.0f * dpiScaleFactor;
    io.Fonts->AddFontDefault(&fontConfig);
#endif
}

void MyImGui::DrawImGuizmo(const std::unique_ptr<Scene>& scene)
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

    for (auto& mesh : scene->meshes_) {

        if (mesh->isSelected) {

            float translation[3];
            float rotation[3];
            float scale[3];
            float objectMatrix[16];

            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(mesh->ubo.model), translation, rotation, scale);
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);
            ImGuizmo::Manipulate(glm::value_ptr(scene->camera_.matrix.view), glm::value_ptr(scene->camera_.matrix.proj),
                                 OP, ImGuizmo::LOCAL, objectMatrix);
            mesh->ubo.model = glm::make_mat4(objectMatrix);
        }
    }
}

void MyImGui::DrawDockSpace()
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
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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
        if (ImGui::BeginMenu("Options")) {
            ImGui::MenuItem("Menu");
            ImGui::Separator();
            if (ImGui::MenuItem("Close", NULL, false, &p_open != NULL))
                p_open = false;
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    ImGui::End();
}

void MyImGui::ControlCamera(const std::unique_ptr<Scene>& scene, GLFWwindow* window)
{
    if (ImGui::IsKeyPressed(ImGuiKey_C) || ImGui::Checkbox("Camera Control [c]", &scene->camera_.isControllable)) {
        scene->camera_.isControllable = !scene->camera_.isControllable;
        ImGuiIO& io = ImGui::GetIO();

        if (scene->camera_.isControllable) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        scene->camera_.isInitial = true;
    }
}

void MyImGui::Draw(const std::unique_ptr<Scene>& scene, GLFWwindow* window)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::NewFrame();

    DrawDockSpace();

    // Object List Window
    ImGui::Begin("List");
    // for (auto& mesh : scene->meshes) {
    //     ImGui::Selectable(mesh->name.c_str(), &mesh->isSelected);
    // }
    static int currentItem = -1;
    const char* items[2] = { "a", "b" };
    ImGui::ListBox("Object", &currentItem, items, IM_ARRAYSIZE(items), 4);
    ImGui::End();

    DrawImGuizmo(scene);

    // Object Attribute Window
    ImGui::Begin("Object Attribute");
    ControlCamera(scene, window);

    for (int i = 0; i < scene->meshes_.size(); i++) {
        if (!scene->meshes_[i]->isSelected)
            continue;

        float translation[3];
        float rotation[3];
        float scale[3];
        float objectMatrix[16];

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(scene->meshes_[i]->ubo.model), translation, rotation, scale);

        std::string id = "##";
        id.append(std::to_string(i));
        std::vector<std::string> labels = { "Translation", "Rotation" };
        ImGui::Text(scene->meshes_[i]->name.c_str());
        ImGui::SliderFloat3(labels[0].append(id).c_str(), translation, -10.0f, 10.0f);
        ImGui::SliderFloat3(labels[1].append(id).c_str(), rotation, -180.0f, 180.0f);

        ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);

        scene->meshes_[i]->ubo.model = glm::make_mat4(objectMatrix);
    }

    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
}