#include "imgui.h"

void MyImGui::Setup(std::weak_ptr<Scene> scene)
{
    scene_ = scene;
    {
        vk::DescriptorPoolSize poolSizes[] = {
            { vk::DescriptorType::eCombinedImageSampler, 1 },
        };

        vk::DescriptorPoolCreateInfo poolInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, static_cast<uint32_t>(IM_ARRAYSIZE(poolSizes)), poolSizes);

        if (Device::GetDevice().createDescriptorPool(&poolInfo, nullptr, &imGuiDescriptorPool) != vk::Result::eSuccess) {
            spdlog::error("failed to create descriptor pool for ImGui");
        }
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(*Window::GetWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Instance::GetInstance();
    init_info.PhysicalDevice = Device::GetPhysicalDevice();
    init_info.Device = Device::GetDevice();
    init_info.QueueFamily = Device::GetQueueFamilyIndices().graphicsFamily.value();
    init_info.Queue = Device::GetGraphicsQueue();
    init_info.DescriptorPool = imGuiDescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = Swapchain::GetSupportDetail().capabilities.minImageCount;
    init_info.ImageCount = Swapchain::GetDetail().frames.size();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, GraphicsPipeline::RenderPass());

#if defined(_WIN32)
    float dpiScaleFactor = ImGui_ImplWin32_GetDpiScaleForHwnd(GetDesktopWindow());

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 13.0f * dpiScaleFactor;
    io.Fonts->AddFontDefault(&fontConfig);
#endif
}

void MyImGui::DrawImGuizmo()
{
    ImGuiIO& io = ImGui::GetIO();

    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene_.lock()->camera.isControllable)
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene_.lock()->camera.isControllable)
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene_.lock()->camera.isControllable)
        OP = ImGuizmo::OPERATION::SCALE;

    // ImGuizmo
    ImGuizmo::BeginFrame();
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);

    for (auto& mesh : scene_.lock()->meshes) {
        if (mesh->isSelected) {

            float translation[3];
            float rotation[3];
            float scale[3];
            float objectMatrix[16];

            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(mesh->ubo.model), translation, rotation, scale);
            ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);
            ImGuizmo::Manipulate(glm::value_ptr(scene_.lock()->camera.matrix.view), glm::value_ptr(scene_.lock()->camera.matrix.proj),
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

void MyImGui::SetCameraControl()
{
    if (ImGui::IsKeyPressed(ImGuiKey_C) || ImGui::Checkbox("Camera Control [c]", &scene_.lock()->camera.isControllable)) {
        scene_.lock()->camera.isControllable = !scene_.lock()->camera.isControllable;
        ImGuiIO& io = ImGui::GetIO();

        if (scene_.lock()->camera.isControllable) {
            glfwSetInputMode(*Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(*Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        scene_.lock()->camera.isInitial = true;
    }
}

void MyImGui::Draw()
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

    DrawImGuizmo();

    // Object Attribute Window
    ImGui::Begin("Object Attribute");
    SetCameraControl();

    for (int i = 0; i < scene_.lock()->meshes.size(); i++) {
        if (!scene_.lock()->meshes[i]->isSelected)
            continue;

        float translation[3];
        float rotation[3];
        float scale[3];
        float objectMatrix[16];

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(scene_.lock()->meshes[i]->ubo.model), translation, rotation, scale);

        std::string id = "##";
        id.append(std::to_string(i));
        std::vector<std::string> labels = { "Translation", "Rotation" };
        ImGui::Text(scene_.lock()->meshes[i]->name.c_str());
        ImGui::SliderFloat3(labels[0].append(id).c_str(), translation, -10.0f, 10.0f);
        ImGui::SliderFloat3(labels[1].append(id).c_str(), rotation, -180.0f, 180.0f);

        ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);

        scene_.lock()->meshes[i]->ubo.model = glm::make_mat4(objectMatrix);
    }

    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
}

MyImGui::~MyImGui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    Device::GetDevice().destroyDescriptorPool(imGuiDescriptorPool);
    ImGui::DestroyContext();
}