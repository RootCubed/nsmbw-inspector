#include <iostream>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glad/glad.h>

#include "../vendor/imgui_impl_glfw.h"
#include "../vendor/imgui_impl_opengl3.h"

#include <iostream>

#define GLSL_VERSION "#version 330"

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(600, 600, "Hello, ImGUI!", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Could not create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    float highDPIscaleFactor = 1.0;
    if (xscale > 1 || yscale > 1) {
        highDPIscaleFactor = xscale;
    }
    std::cout << highDPIscaleFactor << std::endl;

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Could not initialize GLAD!" << std::endl;
        return -1;
    }

    ImGui::CreateContext();


    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = "imgui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(highDPIscaleFactor);
    
    ImFontConfig cfg;
    cfg.SizePixels = 13 * highDPIscaleFactor;
    io.Fonts->AddFontDefault(&cfg);
    cfg.GlyphOffset.y = highDPIscaleFactor;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Window");
        {
            ImGui::Text("NSMBW Inspector");
            ImGui::Button("Click Me!");
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
