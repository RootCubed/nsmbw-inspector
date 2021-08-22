add_executable(nsmbw-inspector)

set(NSMBWINSPECTOR_SRC
    main.cpp
    structure_file.cpp
    displayers.cpp
    DolphinReader/DolphinReader.cpp
    DolphinReader/Dolphin-memory-engine/Source/Common/MemoryCommon.cpp
    DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/DolphinAccessor.cpp
)

if (WIN32)
    list(APPEND NSMBWINSPECTOR_SRC
        DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/Windows/WindowsDolphinProcess.cpp
    )
    target_link_libraries(nsmbw-inspector shcore)
else()
    list(APPEND NSMBWINSPECTOR_SRC
        DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/Linux/LinuxDolphinProcess.cpp
    )
endif()

target_sources(nsmbw-inspector PUBLIC ${NSMBWINSPECTOR_SRC})

target_link_libraries(nsmbw-inspector IMGUI)