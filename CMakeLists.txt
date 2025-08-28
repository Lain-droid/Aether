cmake_minimum_required(VERSION 3.20)
project(Aether LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Find packages
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(spdlog CONFIG REQUIRED)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
    add_compile_definitions(_WIN32_WINNT=0x0A00) # Windows 10
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Build configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_BUILD)
    if(MSVC)
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(-O0 -g)
    endif()
else()
    add_compile_definitions(RELEASE_BUILD)
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O2)
    endif()
endif()

# Include directories
include_directories(src/backend)

# Add subdirectories
add_subdirectory(src/backend)

# Optional: Add tests
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS aether_backend 
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
