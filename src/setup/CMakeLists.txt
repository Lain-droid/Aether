cmake_minimum_required(VERSION 3.20)
project(aether_setup)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Setup source files
set(SETUP_SOURCES
    SetupWizard.cpp
)

# Create Setup EXE
add_executable(aether_setup WIN32 ${SETUP_SOURCES})

# Include directories
target_include_directories(aether_setup PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(aether_setup PRIVATE 
    user32
    gdi32
    comctl32
)

# Set output name
set_target_properties(aether_setup PROPERTIES
    OUTPUT_NAME "AetherSetup"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
)
