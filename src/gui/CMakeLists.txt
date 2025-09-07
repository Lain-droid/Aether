cmake_minimum_required(VERSION 3.20)
project(aether_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static runtime linking to reduce dependencies
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# VSCode-like GUI source files
set(GUI_SOURCES
    VSCodeGUI.cpp
)

# Create GUI DLL
add_library(aether_gui SHARED ${GUI_SOURCES})

# Static linking flags
if(MSVC)
    target_compile_options(aether_gui PRIVATE /MT)
endif()

# Include directories
target_include_directories(aether_gui PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries for VSCode-like interface
target_link_libraries(aether_gui PRIVATE 
    comctl32
    user32
    gdi32
    ole32
    oleaut32
)

# Set output name
set_target_properties(aether_gui PROPERTIES
    OUTPUT_NAME "AetherGUI"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
)
