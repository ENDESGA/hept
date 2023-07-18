cmake_minimum_required(VERSION 3.20)

# General project setup
project(hept C)
set(CMAKE_C_STANDARD 99)

# Use target-specific compile features instead of global C flags
add_compile_options(
  "$<$<CONFIG:RELEASE>:-march=native>"
  "$<$<CONFIG:DEBUG>:-Og;-g>"
)

# Output directories for different build types
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out/Release)

# Available build configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Suppress the ZERO_CHECK target generation
set(CMAKE_SUPPRESS_REGENERATION TRUE)

# Directories
set(SOURCE_DIR "src")
set(INCLUDE_DIR "inc")

# Search for all source files
file(GLOB SOURCES "${SOURCE_DIR}/*.c")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

# Vulkan setup
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

# X11 setup for Unix systems excluding Apple
if (UNIX AND NOT APPLE)
  find_package(X11 REQUIRED)
  target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})
  target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES})
endif ()

# Custom commands to generate git revision info
find_package(Git)
if(GIT_FOUND)
  add_custom_target(git_revision
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  add_dependencies(${PROJECT_NAME} git_revision)

  target_compile_definitions(${PROJECT_NAME} PRIVATE GIT_REVISION="${GIT_REVISION}")
endif()

# Set file extension based on the system
if (WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".exe")
elseif (UNIX)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bin")
endif ()
