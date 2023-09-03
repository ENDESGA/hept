cmake_minimum_required(VERSION 3.20)

project(hept C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_SUPPRESS_REGENERATION TRUE)

set(SOURCE_DIR "src")
set(INCLUDE_DIR "inc")

set(SOURCES
        ${SOURCE_DIR}/main.c
        # ... add other source files
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
  target_compile_options(${PROJECT_NAME} PRIVATE
          "$<$<CONFIG:RELEASE>:/Ox;/GT;/Gy;/fp:fast;/GS->"
          "$<$<CONFIG:DEBUG>:/Zi;/Od>"
  )
else()
  target_compile_options(${PROJECT_NAME} PRIVATE
          "$<$<CONFIG:RELEASE>:-O3;-march=native;-funroll-loops;-fexpensive-optimizations;-finline-functions;-fprefetch-loop-arrays;-ffast-math>"
          "$<$<CONFIG:DEBUG>:-Og;-g>"
  )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

find_package(X11 REQUIRED)
if(X11_FOUND)
  target_link_libraries(${PROJECT_NAME} PRIVATE X11::X11)
endif()

if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".exe")
elseif(UNIX)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".bin")
endif()
