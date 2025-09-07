cmake_minimum_required(VERSION 3.20)
project(aether_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static runtime linking
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
  add_compile_options(/MP /EHsc /bigobj /MT)
endif()

# Backend source files
file(GLOB_RECURSE BACKEND_SOURCES
    "*.cpp"
    "*.h"
)

# Create backend DLL
add_library(aether_backend SHARED ${BACKEND_SOURCES})

if (MSVC)
  target_compile_options(aether_backend PRIVATE /MP /EHsc /bigobj)
  target_link_libraries(aether_backend PRIVATE ws2_32 ntdll)
  set_target_properties(aether_backend PROPERTIES OUTPUT_NAME "aether_backend" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
