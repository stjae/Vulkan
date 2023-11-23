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
    (void)io;
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

void MyImGui::Draw(Camera& camera, const std::unique_ptr<Scene>& scene)
{
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

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
    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &p_open, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);
    // Submit the DockSpace
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Options")) {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Menu");
            ImGui::Separator();
            if (ImGui::MenuItem("Close", NULL, false, &p_open != NULL))
                p_open = false;
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
    ImGui::End();

    ImGui::Begin("Object Attribute");

    // if (ImGui::Checkbox("Camera Control", &camera.isControllable)) {
    //     camera.isInitial = true;
    // }
    for (int i = 0; i < scene->meshes.size(); i++) {
        std::string id = "##";
        id.append(std::to_string(i));
        std::vector<std::string> labels = { "Translation", "Rotation" };
        ImGui::SliderFloat3(labels[0].append(id).c_str(), &scene->meshes[i]->pos[0], -10.0f, 10.0f);
        ImGui::SliderFloat3(labels[1].append(id).c_str(), &scene->meshes[i]->rotation[0], -180.0f, 180.0f);
    }

    ImGui::End();

    ImGui::Begin("Object List");
    ImGui::End();

    // ImGuizmo
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);

    for (auto& mesh : scene->meshes) {

        ImGuizmo::Manipulate(glm::value_ptr(camera.matrix.view), glm::value_ptr(camera.matrix.proj),
                             ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(mesh->ubo.model));
    }

    ImGui::EndFrame();
    ImGui::Render();
}