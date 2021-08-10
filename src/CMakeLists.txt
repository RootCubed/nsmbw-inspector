add_executable(nsmbw-inspector)

if (WIN32)
target_sources(nsmbw-inspector
    PUBLIC
        main.cpp
        structure_file.cpp
        DolphinReader/DolphinReader.cpp
        DolphinReader/Dolphin-memory-engine/Source/Common/MemoryCommon.cpp
        DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/DolphinAccessor.cpp
        DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/Windows/WindowsDolphinProcess.cpp)
else()
target_sources(nsmbw-inspector
    PUBLIC
        main.cpp
        structure_file.cpp
        DolphinReader/DolphinReader.cpp
        DolphinReader/Dolphin-memory-engine/Source/Common/MemoryCommon.cpp
        DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/DolphinAccessor.cpp
        DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/Linux/LinuxDolphinProcess.cpp)
endif()

target_link_libraries(nsmbw-inspector IMGUI)