#include "imgui.h"
extern size_t dynamicBufferAlignment;

void MyImGui::Setup(std::weak_ptr<Scene> scene, GraphicsPipeline& pipeline)
{
    scene_ = scene;
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

void MyImGui::DrawImGuizmo(int currentItem)
{
    ImGuiIO& io = ImGui::GetIO();

    static ImGuizmo::OPERATION OP(ImGuizmo::OPERATION::TRANSLATE);
    if (ImGui::IsKeyPressed(ImGuiKey_W) && !scene_.lock()->camera->isControllable)
        OP = ImGuizmo::OPERATION::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E) && !scene_.lock()->camera->isControllable)
        OP = ImGuizmo::OPERATION::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R) && !scene_.lock()->camera->isControllable)
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

    glm::mat4* modelMat = (glm::mat4*)((uint64_t)scene_.lock()->uboDataDynamic.model + (currentItem * dynamicBufferAlignment));

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(*modelMat), translation,
                                          rotation, scale);
    ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, objectMatrix);
    ImGuizmo::Manipulate(glm::value_ptr(scene_.lock()->camera->matrix_.view),
                         glm::value_ptr(scene_.lock()->camera->matrix_.proj), OP,
                         ImGuizmo::LOCAL, objectMatrix);
    *modelMat = glm::make_mat4(objectMatrix);
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
            ImGui::MenuItem("Open");
            ImGui::Separator();
            if (ImGui::MenuItem("Square")) {
            }
            if (ImGui::MenuItem("Cube")) {
                scene_.lock()->meshes.emplace_back(std::make_shared<Mesh>());
                scene_.lock()->meshes.back()->CreateCube(nullptr);
                scene_.lock()->meshes.back()->CreateBuffers();
                scene_.lock()->PrepareMeshes();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    ImGui::End();
}

void MyImGui::SetCameraControl()
{
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        scene_.lock()->camera->isControllable = !scene_.lock()->camera->isControllable;
        ImGuiIO& io = ImGui::GetIO();

        if (scene_.lock()->camera->isControllable) {
            glfwSetInputMode(*Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(*Window::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }

        scene_.lock()->camera->isInitial = true;
    }
}

void MyImGui::Draw()
{
    SetCameraControl();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::NewFrame();

    DrawDockSpace();

    // Object List Window
    static int currentItem = -1;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() || currentItem > scene_.lock()->meshes.size() - 1) // Prevent index out of vector range
        currentItem = -1;

    ImGui::BeginListBox("Object List");
    for (int i = 0; i < scene_.lock()->meshes.size(); i++) {
        if (ImGui::Selectable(scene_.lock()->meshes[i]->name.c_str(), scene_.lock()->meshes[i]->isSelected))
            currentItem = i;
    }
    ImGui::EndListBox();

    // Object Attribute Window
    ImGui::Begin("Object Attribute");
    if (currentItem > -1) {
        DrawImGuizmo(currentItem);

        glm::mat4* modelMat = (glm::mat4*)((uint64_t)scene_.lock()->uboDataDynamic.model + (currentItem * dynamicBufferAlignment));

        float translation[3];
        float rotation[3];
        float scale[3];
        float objectMatrix[16];

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(*modelMat), translation,
                                              rotation, scale);

        std::string id = "##";
        id.append(scene_.lock()->meshes[currentItem]->name.c_str());
        std::vector<std::string> labels = { "Translation", "Rotation" };
        ImGui::Text(scene_.lock()->meshes[currentItem]->name.c_str());
        ImGui::SliderFloat3(labels[0].append(id).c_str(), translation, -10.0f, 10.0f);
        ImGui::SliderFloat3(labels[1].append(id).c_str(), rotation, -180.0f, 180.0f);

        ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale,
                                                objectMatrix);

        *modelMat = glm::make_mat4(objectMatrix);
    }
    ImGui::End();

    // Information Overlay
    ImGui::SetNextWindowPos(ImVec2(viewport->Size.x, 0.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("Information", nullptr, window_flags);
    ImGui::Text("%s", GetFrameRate().c_str());
    ImGui::Text("Camera Control: %s [press C]", scene_.lock()->camera->isControllable ? "on" : "off");
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