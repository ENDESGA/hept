cmake_minimum_required(VERSION 3.21)
project(hept)

set(CMAKE_CXX_STANDARD 20)

add_executable(hept main.cpp hept.h)
target_link_libraries(hept SDL2 SDL2main SDL2_image SDL2_gfx GL GLEW)