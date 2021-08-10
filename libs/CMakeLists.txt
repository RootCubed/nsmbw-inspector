# SDL2
set(SDL2_DIR "SDL")
add_subdirectory(${SDL2_DIR})


# OpenGL
find_package(OpenGL REQUIRED)

set(GL3W_DIR "imgui/examples/libs/gl3w")
add_library(GL3W STATIC)

target_sources(GL3W PRIVATE ${GL3W_DIR}/GL/gl3w.c)
target_include_directories(GL3W PUBLIC ${GL3W_DIR})
target_link_libraries(GL3W PUBLIC ${OPENGL_LIBRARIES})

# imgui
set(IMGUI_DIR "imgui")
add_library(IMGUI STATIC)

target_sources(IMGUI
    PRIVATE
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui.cpp

    PRIVATE
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
)

target_include_directories(IMGUI
    PUBLIC ${IMGUI_DIR}
    PUBLIC ${IMGUI_DIR}/backends
    PUBLIC ${SDL_DIR}/include
)

target_compile_definitions(IMGUI PUBLIC -DIMGUI_IMPL_OPENGL_LOADER_GL3W)
target_link_libraries(IMGUI PUBLIC GL3W)

target_link_libraries(IMGUI PUBLIC SDL2main SDL2 ${CMAKE_DL_LIBS})