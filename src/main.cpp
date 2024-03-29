#include <iostream>

#include <imgui.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <SDL.h>

#include <GL/gl3w.h>

#include "DolphinReader/DolphinReader.h"

#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <vector>
#include <map>

#include "helper.h"
#include "structure_file.h"

#include "nsmbw/mj2d/f/f_manager.h"

#ifdef WIN32
#include <Windows.h>
#include <ShellScalingAPI.h>
#endif

#define GLSL_VERSION "#version 330"

// define some macros for quick access to dolphinreader
#define read(addr, type) *(type *) DolphinReader::readValues((u32) (addr), sizeof(type))
#define readU32 DolphinReader::readU32
#define readU16 DolphinReader::readU16
#define readU8 DolphinReader::readU8
#define writeU32 DolphinReader::writeU32
#define writeU16 DolphinReader::writeU16
#define writeU8 DolphinReader::writeU8
#define readStr(addr, buf) void *tmp = DolphinReader::readValues((u32) (addr), sizeof(buf));\
memcpy((void *) buf, tmp, sizeof(buf))

int status = 0;

enum gameVer {
    PALv1, PALv2,
    NTSCv1, NTSCv2,
    JPNv1, JPNv2,
    VER_UNKNOWN
};

const std::map<gameVer, u32> listPtrs {
    {PALv1, 0x80377d48}, {PALv2, 0x80377d48},
    {NTSCv1, 0x80377d10}, {NTSCv2, 0x80377a10},
    {JPNv1, 0x80377790}, {JPNv2, 0x80377790},
    {VER_UNKNOWN, 0x0}
};

gameVer currVersion = VER_UNKNOWN;
bool isOtherNode = false;

void DrawStartupView();
void InitMainView();
void DrawMainView();

StructureFile structures;

int main(int argc, char **argv) {
    #ifdef WIN32
    SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
    #endif
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL Init Error: %s\n", SDL_GetError());
        return -1;
    }
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("NSMBW Inspector", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    gl3wInit();

    float dpi;
    if (SDL_GetDisplayDPI(0, &dpi, NULL, NULL) != 0) {
        fprintf(stderr, "Failed to obtain DPI information for display 0: %s\n", SDL_GetError());
        return -1;
    }
    float highDPIscaleFactor = round(dpi / 96.0f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = "imgui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(highDPIscaleFactor);
    style.WindowRounding = 6;

    ImFontConfig cfg;
    cfg.SizePixels = 14 * highDPIscaleFactor;
    io.Fonts->AddFontFromFileTTF("iosevka-ss01-regular.ttf", 14 * highDPIscaleFactor, &cfg);

    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("sarasa-term-j-regular.ttf", 14 * highDPIscaleFactor, &cfg, io.Fonts->GetGlyphRangesJapanese());
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

    bool shouldClose = false;
    while (!shouldClose) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                shouldClose = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                shouldClose = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (structuresErrorPopup) {
            ImGui::OpenPopup("Error loading structures");
        }

        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5, 0.5));
        ImGui::SetNextWindowSize(ImVec2(500, 0));

        if (ImGui::BeginPopupModal("Error loading structures", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextWrapped("%s", errorText.c_str());
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                shouldClose = true;
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
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
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
        ImGui::Text("%s", s.c_str());
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
            case DolphinComm::DolphinAccessor::DolphinStatus::hooked: {
                status = 1;
                    u16 offs0 = readU32(0x80768D50) & 0xFFFF;
                    u16 offs40 = readU32(0x80768D90) & 0xFFFF;
                    switch (offs0) {
                        case 0x6DE1:
                            currVersion = PALv1;
                            isOtherNode = false;
                            break;
                        case 0x6CA1:
                            currVersion = NTSCv1;
                            isOtherNode = true;
                            break;
                        case 0x6AB1:
                            currVersion = JPNv1;
                            isOtherNode = true;
                            break;
                        default:
                            switch (offs40) {
                                case 0x6DA1:
                                    currVersion = PALv2;
                                    isOtherNode = true;
                                    break;
                                case 0x6C61:
                                    currVersion = NTSCv2;
                                    isOtherNode = true;
                                    break;
                                case 0x6A71:
                                    currVersion = JPNv2;
                                    isOtherNode = true;
                                    break;
                                default:
                                    currVersion = VER_UNKNOWN;
                                    status = 0;
                            }
                    }
                if (status == 1) {
                    ImGui::Text("Hooked to Dolphin! Waiting for draw list to show up in memory...");
                } else {
                    ImGui::Text("Unknown game version found.");
                }
                break;
            }
            case DolphinComm::DolphinAccessor::DolphinStatus::unHooked:
                ImGui::Text("Not hooked to Dolphin yet.");
                break;
        }
    }
    ImGui::End();
}

std::vector<u32> readList(u32 address) {
    std::vector<u32> res(0);
    PTMFList drawMng = read(address, PTMFList);
    if (!isOtherNode) {
        fLiNdBa_c_v1 currNode = read(_byteswap_ulong(drawMng.start), fLiNdBa_c_v1);
        int i = 0;
        while (currNode.next != 0 && i < 500) {
            res.push_back(_byteswap_ulong(currNode.thisobj));
            currNode = read(_byteswap_ulong(currNode.next), fLiNdBa_c_v1);
            i++;
        }
    } else {
        fLiNdBa_c_v2 currNode = read(_byteswap_ulong(drawMng.start), fLiNdBa_c_v2);
        int i = 0;
        while ((currNode.next != 0 || currNode.otherNext != 0) && i < 500) {
            res.push_back(_byteswap_ulong(currNode.thisobj));
            if (currNode.next == 0) {
                currNode = read(_byteswap_ulong(currNode.otherNext), fLiNdBa_c_v2);
            } else {
                currNode = read(_byteswap_ulong(currNode.next), fLiNdBa_c_v2);
            }
            i++;
        }
    }
    return res;
}

u32 selectedInstance = 0x0;
char selectedInstanceString[128];
bool selectedInstanceExists = false;
bool selectedInstanceValidType = false;

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

            std::vector<u32> list = readList(listPtrs.at(currVersion));
            char nameBuf[64];
            bool foundSelectedInstance = false;
            for (auto el : list) {
                u32 namePtr = _byteswap_ulong(read(el + 0x6c, u32));
                readStr(namePtr, nameBuf);

                char selectableNameBuf[128];
                sprintf(selectableNameBuf, "%s##%08x", nameBuf, el);
                selectedInstanceExists = (el == selectedInstance && strcmp(selectedInstanceString, selectableNameBuf) == 0);

                bool wasSelected = ImGui::Selectable(selectableNameBuf, selectedInstanceExists);
                if (wasSelected || selectedInstanceExists) {
                    selectedInstance = el;
                    strcpy(selectedInstanceString, selectableNameBuf);
                    Structure *s = structures.getStruct(nameBuf);

                    selectedInstanceValidType = (s != NULL);
                    if (selectedInstanceValidType) selected.setType(s);

                    selectedInstanceExists = true;
                }
                foundSelectedInstance |= selectedInstanceExists;
            }
            selectedInstanceExists = foundSelectedInstance;
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        char name[128];
        u32 namePtr = _byteswap_ulong(read(selectedInstance + 0x6c, u32));
        readStr(namePtr, name);

        ImGui::BeginChild("Inspector", ImVec2(0, 0), true);
        {
            ImGui::Checkbox("Show all structs/fields", &selected.showAllFields);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth() / 2, 0));
                ImGui::BeginTooltip();
                ImGui::TextWrapped("Shows/hides the fields which are not defined in a displayer for a class. If there are none to show, the class itself also gets hidden.");
                ImGui::EndTooltip();
            }
            
            ImGui::Separator();

            if (!selectedInstanceExists) {
                ImGui::TextWrapped("Select an instance to view its properties.");
            } else if (!selectedInstanceValidType) {
                ImGui::TextWrapped("Actor %s is not defined in structures.txt yet.", name);
            } else {
                // render instance viewer
                ImGui::TextWrapped("%s @ 0x%08x", name, readU32(selectedInstance + 0x6c));
                ImGui::Separator();
                selected.drawInstance(selectedInstance);
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}