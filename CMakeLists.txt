cmake_minimum_required(VERSION 3.20)

project(hept C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(SOURCE_DIR "/src")
set(INCLUDE_DIR "/inc")

file(GLOB SOURCES "${SOURCE_DIR}/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".exe")
elseif (UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bin")
endif ()
