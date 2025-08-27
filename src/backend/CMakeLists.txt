cmake_minimum_required(VERSION 3.20)
project(aether_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Force x64
if (NOT CMAKE_GENERATOR_PLATFORM)
  set(CMAKE_GENERATOR_PLATFORM x64 CACHE STRING "" FORCE)
endif()

file(GLOB_RECURSE SRC
  *.cpp
)

add_library(aether_backend STATIC ${SRC})
target_include_directories(aether_backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
  target_link_libraries(aether_backend PRIVATE ws2_32)
endif()
