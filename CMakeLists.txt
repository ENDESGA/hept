cmake_minimum_required(VERSION 3.20)

# Project configuration
project(hept C)
set(CMAKE_C_STANDARD 99)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_SUPPRESS_REGENERATION TRUE)

# Set global compiler flags for optimization
add_compile_options(
        "$<$<CONFIG:RELEASE>:-O3;-march=native;-funroll-loops;-fexpensive-optimizations;-finline-functions;-fprefetch-loop-arrays;-ffast-math>"
)

# Directories
set(SOURCE_DIR "src")
set(INCLUDE_DIR "inc")

# Locate all source files
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.c")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

# Add Vulkan libraries
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# Add X11 libraries for Unix systems excluding Apple
if (UNIX AND NOT APPLE)
  find_package(X11 REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE X11::X11)
endif ()

# Define executable suffix based on OS
if (WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".exe")
elseif (UNIX)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bin")
endif ()
