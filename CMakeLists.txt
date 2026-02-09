cmake_minimum_required(VERSION 3.10)
project(LearnOpenGL)

set(CMAKE_CXX_STANDARD 17)

add_executable(LearnOpenGL
src/main.cpp
src/glad.c
src/stb_image.cpp
Dev/ImGuizmo/ImGuizmo.cpp
Dev/nfd/src/nfd_win.cpp
Dev/nfd/src/nfd_common.c
Dev/nfd/src/nfd_win.cpp
Dev/imgui-node-editor/crude_json.cpp
Dev/imgui-node-editor/imgui_canvas.cpp
Dev/imgui-node-editor/imgui_node_editor_api.cpp
Dev/imgui-node-editor/imgui_node_editor.cpp
)

target_compile_definitions(LearnOpenGL PRIVATE IMGUI_DEFINE_MATH_OPERATORS)
target_compile_definitions(LearnOpenGL PRIVATE SHADERGRAPH_ENABLE_NODE_EDITOR)


target_include_directories(LearnOpenGL PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/Dev/glm"
    "${CMAKE_SOURCE_DIR}/Dev/ImGuizmo"
    "${CMAKE_SOURCE_DIR}/Dev/nfd/src/include"
    "${CMAKE_SOURCE_DIR}/Dev/imgui-node-editor"
)

add_subdirectory(Dev/glfw-3.4)
add_subdirectory(Dev/imgui-1.92.5)
add_subdirectory(Dev/assimp-6.0.2)

target_link_libraries(LearnOpenGL PRIVATE
    glfw
    opengl32
    imgui
    assimp
)