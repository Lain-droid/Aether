cmake_minimum_required(VERSION 3.20)
project(aether_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static runtime linking to reduce dependencies
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
  add_compile_options(/MP /EHsc /bigobj /MT)
endif()

# Avoid forcing generator platform; respect host toolchain

file(GLOB_RECURSE SRC
  *.cpp
)

if (WIN32)
  add_library(aether_backend SHARED ${SRC})
else()
  add_library(aether_backend STATIC ${SRC})
endif()
target_include_directories(aether_backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
  target_link_libraries(aether_backend PRIVATE ws2_32)
  set_target_properties(aether_backend PROPERTIES OUTPUT_NAME "aether_backend" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
