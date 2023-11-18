#include "baseApp.h"

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
        camera->isControllable = !camera->isControllable;

        if (camera->isControllable) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        camera->isInitial = true;
    }
}

Application::Application(const int width, const int height, const char* wName)
    : window(width, height, wName), camera(window.window)
{
    scene = std::make_unique<Scene>();
    engine = std::make_unique<GraphicsEngine>(width, height, window.window, scene);

    SetupImGui();

    engine->InitSwapchainImages();
    engine->Prepare(scene);

    glfwSetKeyCallback(window.window, KeyCallback);
}

void Application::SetupImGui()
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

    // ImFontConfig fontConfig;
    // fontConfig.SizePixels = 20.0f;
    // io.Fonts->AddFontDefault(&fontConfig);

    {
        vk::CommandBufferAllocateInfo allocateInfo(engine->command.commandPool, vk::CommandBufferLevel::ePrimary, 1);

        vk::CommandBuffer commandBuffer;
        auto allocateResult = engine->device.vkDevice.allocateCommandBuffers(&allocateInfo, &commandBuffer);

        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);

        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

        vk::SubmitInfo submitInfo({}, {}, {}, 1, &commandBuffer, {}, {});
        commandBuffer.end();
        auto submitResult = engine->device.vkGraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);

        engine->device.vkDevice.waitIdle();
        engine->device.vkDevice.freeCommandBuffers(engine->command.commandPool, 1, &commandBuffer);

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window.window)) {
        glfwPollEvents();

        if (camera.isControllable) {
            camera.Update(window);
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Hello");
        if (ImGui::Checkbox("Camera Control", &camera.isControllable)) {
            camera.isInitial = true;
        }
                ImGui::End();
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        engine->Render(scene, draw_data, camera);
        GetFramerate();
    }
}

void Application::GetFramerate()
{
    static int frameCount;
    static double currentTime, lastTime;

    currentTime = glfwGetTime();
    delta = currentTime - lastTime;

    if (delta > 1.0) {
        std::stringstream title;
        title << frameCount << " fps, " << 1000.0f / frameCount << " ms";
        glfwSetWindowTitle(window.window, title.str().c_str());

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;
}