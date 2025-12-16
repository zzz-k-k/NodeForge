cmake_minimum_required(VERSION 3.10)
project(LearnOpenGL)

set(CMAKE_CXX_STANDARD 17)

add_executable(LearnOpenGL
    src/main.cpp
    src/glad.c
    src/stb_image.cpp
)

target_include_directories(LearnOpenGL PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/Dev/glm"
)

add_subdirectory(Dev/glfw-3.4)
add_subdirectory(Dev/imgui-1.92.5)

target_link_libraries(LearnOpenGL PRIVATE
    glfw
    opengl32
    imgui
)