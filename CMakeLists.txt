cmake_minimum_required(VERSION 3.20)

set(CMAKE_SUPPRESS_REGENERATION TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

project("hept" C)

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/inc")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

file(GLOB SOURCES "${SOURCE_DIR}/*.c")

include_directories(${INCLUDE_DIR})
add_executable(${PROJECT_NAME} ${SOURCES})

IF (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".exe")
ELSEIF (UNIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bin")
ENDIF ()
