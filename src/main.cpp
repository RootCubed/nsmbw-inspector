#include <iostream>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <glad/glad.h>

#include "../vendor/imgui_impl_glfw.h"
#include "../vendor/imgui_impl_opengl3.h"

#include "../DolphinReader/DolphinReader.h"

#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <vector>

#include "structure_file.h"

#include "nsmbw/mj2d/f/f_manager.h"

#define GLSL_VERSION "#version 330"

int status = 0;

void DrawStartupView();
void InitMainView();
void DrawMainView();

StructureFile structures;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(1200, 800, "NSMBW Inspector", nullptr, nullptr);
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
        highDPIscaleFactor = ceil(xscale);
    }
    std::cout << highDPIscaleFactor << std::endl;

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Could not initialize GLAD!" << std::endl;
        return -1;
    }

    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = "imgui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(highDPIscaleFactor);
    style.WindowRounding = 6;
    
    ImFontConfig cfg;
    cfg.SizePixels = 13 * highDPIscaleFactor;
    io.Fonts->AddFontFromFileTTF("iosevka-ss01-regular.ttf", 13 * highDPIscaleFactor, &cfg);
    io.Fonts->Build();
    cfg.GlyphOffset.y = highDPIscaleFactor;

    bool structuresErrorPopup = false;
    std::string errorText;
    
    std::string structureFileBuffer;
    try {
        std::ifstream f;
        f.open("structures.txt");
        std::string line;
        while (std::getline(f, line)) {
            structureFileBuffer.append(line);
            structureFileBuffer.append("\n");
        }
        f.close();
    } catch (std::exception e) {
        errorText = e.what();
        structuresErrorPopup = true;
    }
    
    try {
        structures = StructureFile(structureFileBuffer);
    } catch (StructureFileException e) {
        errorText = e.what();
        structuresErrorPopup = true;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (structuresErrorPopup) {
            ImGui::OpenPopup("Error loading structures");
        }

        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5, 0.5));
        ImGui::SetNextWindowSize(ImVec2(500, 0));

        if (ImGui::BeginPopupModal("Error loading structures", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextWrapped(errorText.c_str());
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(window, true);
                structuresErrorPopup = false;
            }
            ImGui::EndPopup();
        }

        if (status == 0) {
            DrawStartupView();
        } else {
            DrawMainView();
        }

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

DolphinComm::DolphinAccessor::DolphinStatus startupWindow_status = DolphinComm::DolphinAccessor::DolphinStatus::unHooked;

void DrawStartupView() {
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("nsmbw-inspector", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    {
        std::string s = "NSMBW Inspector";
        float font_size = ImGui::GetFontSize() * s.size() / 2;
        ImGui::SameLine(
            ImGui::GetWindowSize().x / 2 -
            font_size + (font_size / 2)
        );
        ImGui::Text(s.c_str());
        if (ImGui::Button("Hook to Dolphin")) {
            startupWindow_status = DolphinReader::hook();
        }
        switch (startupWindow_status) {
            case DolphinComm::DolphinAccessor::DolphinStatus::noEmu:
                ImGui::Text("There is no game started. Please launch a game and try again.");
                break;
            case DolphinComm::DolphinAccessor::DolphinStatus::notRunning:
                ImGui::Text("Dolphin is not running. Please start Dolphin and try again.");
                break;
            case DolphinComm::DolphinAccessor::DolphinStatus::hooked:
                ImGui::Text("Hooked to Dolphin!");
                status = 1;
                break;
            case DolphinComm::DolphinAccessor::DolphinStatus::unHooked:
                ImGui::Text("Not hooked to Dolphin yet.");
                break;
        }
    }
    ImGui::End();
}

void gen_random(char *s, int l) {
    for (int c; c=rand()%62, *s++ = (c+"07="[(c+16)/26]), l-->1;);
}

#define read(addr, type) *(type *) DolphinReader::readValues((u32) (addr), sizeof(type))
#define readU32 DolphinReader::readU32
#define readU16 DolphinReader::readU16
#define readU8 DolphinReader::readU8
#define writeU32 DolphinReader::writeU32
#define writeU16 DolphinReader::writeU16
#define writeU8 DolphinReader::writeU8
#define readStr(addr, buf) void *tmp = DolphinReader::readValues((u32) (addr), sizeof(buf));\
memcpy((void *) buf, tmp, sizeof(buf))

std::vector<u32> readList(u32 address) {
    std::vector<u32> res(0);
    PTMFList drawMng = read(address, PTMFList);
    fLiNdBa_c currNode = read(_byteswap_ulong(drawMng.start), fLiNdBa_c);
    int i = 0;
    while (currNode.next != 0 && i < 500) {
        res.push_back(_byteswap_ulong(currNode.thisobj));
        currNode = read(_byteswap_ulong(currNode.next), fLiNdBa_c);
        i++;
    }
    return res;
}

u32 selectedInstance = 0x0;
bool selectedInstanceExists = false;

StructureInstance selected;

void DrawMainView() {
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("List View", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    {
        ImGui::BeginChild("Instances", ImVec2(400, 0), true);
        {
            ImGui::Text("Instances");
            ImGui::PushStyleColor(ImGuiCol_ChildBg, 0xff404040);
            ImGui::BeginChild("ListBoxInstances", ImVec2(0, 0));
            std::vector<u32> list = readList(0x80377d48);
            char nameBuf[64];
            bool foundSelectedInstance = false;
            for (auto el : list) {
                u32 namePtr = _byteswap_ulong(read(el + 0x6c, u32));
                readStr(namePtr, nameBuf);
                char windowNameBuf[64];
                sprintf(windowNameBuf, "%s##%08x", nameBuf, el);
                if (ImGui::Selectable(windowNameBuf, el == selectedInstance)) {
                    selectedInstance = el;
                    selectedInstanceExists = true;
                    auto s = structures.getStruct(nameBuf);
                    if (s != NULL) {
                        selected.setType(s);
                    }
                }
                foundSelectedInstance |= el == selectedInstance;
            }
            selectedInstanceExists &= foundSelectedInstance;
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        char name[128];
        u32 namePtr = _byteswap_ulong(read(selectedInstance + 0x6c, u32));
        readStr(namePtr, name);
        
        int rS = selected.getReadSize();
        if (rS != -1) {
            std::vector<char> data(rS, 0);
            void *tmp = DolphinReader::readValues(selectedInstance, rS);
            std::memcpy(&data[0], tmp, rS);
            selected.updateData(data);
        }
        ImGui::BeginChild("Inspector", ImVec2(0, 0), true);
        {
            if (!selectedInstanceExists) {
                ImGui::TextWrapped("Select an instance to view its properties.");
            } else {
                // render instance viewer
                ImGui::TextWrapped("%s @ 0x%08x", name, readU32(selectedInstance + 0x6c));
                ImGui::Separator();
                if (rS != -1) {
                    selected.renderInstance();
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}