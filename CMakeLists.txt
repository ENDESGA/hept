cmake_minimum_required(VERSION 3.20)

# Project configuration
project(hept C)
set(CMAKE_C_STANDARD 17)

# Set global compiler flags for optimization
add_compile_options(
  "$<$<CONFIG:RELEASE>:-O3;-march=native;-flto;-fomit-frame-pointer>"
  "$<$<CONFIG:DEBUG>:-Og;-g>"
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
  target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
endif ()

# Custom command to capture git revision
find_package(Git)
if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  target_compile_definitions(${PROJECT_NAME} PRIVATE GIT_REVISION="${GIT_REVISION}")
endif()

# Define executable suffix based on OS
if (WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".exe")
elseif (UNIX)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bin")
endif ()
