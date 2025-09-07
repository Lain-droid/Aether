cmake_minimum_required(VERSION 3.20)
project(Aether)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(src/backend)
add_subdirectory(src/gui)
add_subdirectory(src/setup)
add_subdirectory(src/tools)

# Set startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT aether_setup)
