cmake_minimum_required(VERSION 3.20)
project(aether_tools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static runtime linking
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Dependency checker tool
add_executable(dependency_checker DependencyChecker.cpp)

# Static linking
if(MSVC)
    target_compile_options(dependency_checker PRIVATE /MT)
endif()

# Link libraries
target_link_libraries(dependency_checker PRIVATE 
    advapi32
    kernel32
)

# Set output name
set_target_properties(dependency_checker PROPERTIES
    OUTPUT_NAME "AetherCheck"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
)
