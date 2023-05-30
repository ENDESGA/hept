cmake_minimum_required(VERSION 3.20)

set(CMAKE_SUPPRESS_REGENERATION TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

set(PROJECT_NAME "NIN" CACHE STRING "PROJECT_NAME")
set(SOURCES "../src/main.c" CACHE STRING "SOURCES")
set(INCLUDE "../inc/")

project(${PROJECT_NAME} C)

include_directories(${INCLUDE})
link_directories(${INCLUDE})
add_executable(${PROJECT_NAME} ${SOURCES})

IF (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".exe")
ELSEIF (UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bin")
ENDIF ()
