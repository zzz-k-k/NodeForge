cmake_minimum_required(VERSION 3.10)
project(LearnOpenGL)

set(CMAKE_CXX_STANDARD 17)

add_executable(LearnOpenGL
    src/main.cpp
    src/glad.c
)

target_include_directories(LearnOpenGL PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/Dev/glfw-3.4/include"
)

target_link_directories(LearnOpenGL PRIVATE
    "${CMAKE_SOURCE_DIR}/Dev/glfw-3.4/build/src/Debug"
)

target_link_libraries(LearnOpenGL PRIVATE
    glfw3
    opengl32
)