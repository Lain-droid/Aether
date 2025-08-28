cmake_minimum_required(VERSION 3.20)
project(Aether LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Performance and security options
option(ENABLE_LTO "Enable Link Time Optimization" ON)
option(ENABLE_PGO "Enable Profile Guided Optimization" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_CFI "Enable Control Flow Integrity" OFF)
option(SECURITY_HARDENED "Enable hardened security build" ON)
option(SECURITY_MAXIMUM "Enable maximum security (may impact performance)" OFF)
option(OPTIMIZATION_LEVEL "Optimization level" "aggressive")

# Enable testing
enable_testing()

# Find packages
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(spdlog CONFIG REQUIRED)

# Advanced compiler and security flags
if(MSVC)
    # Base MSVC flags
    add_compile_options(/W4 /WX /permissive- /std:c++20)
    add_compile_definitions(_WIN32_WINNT=0x0A00) # Windows 10
    
    # Security hardening flags
    if(SECURITY_HARDENED OR SECURITY_MAXIMUM)
        add_compile_options(
            /guard:cf          # Control Flow Guard
            /Qspectre          # Spectre mitigation
            /SDL               # Security Development Lifecycle checks
            /GS                # Buffer security check
        )
        
        add_link_options(
            /GUARD:CF          # Control Flow Guard for linking
            /CETCOMPAT         # CET (Control-flow Enforcement Technology) compatible
            /HIGHENTROPYVA     # High entropy ASLR
            /NXCOMPAT          # Data Execution Prevention
            /DYNAMICBASE       # Address Space Layout Randomization
        )
    endif()
    
    # Maximum security flags
    if(SECURITY_MAXIMUM)
        add_compile_options(
            /RTC1              # Runtime checks (debug builds)
            /analyze           # Code analysis
        )
        add_compile_definitions(FORTIFY_SOURCE=3)
    endif()
    
else()
    # GCC/Clang flags
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    
    # Security hardening flags
    if(SECURITY_HARDENED OR SECURITY_MAXIMUM)
        add_compile_options(
            -fstack-protector-strong    # Stack protection
            -fPIE                       # Position Independent Executable
            -fcf-protection=full        # Control Flow protection
            -fstack-clash-protection    # Stack clash protection
        )
        
        add_link_options(
            -Wl,-z,relro,-z,now        # RELRO protection
            -Wl,-z,noexecstack         # Non-executable stack
            -pie                       # Position Independent Executable
        )
    endif()
    
    # Maximum security flags
    if(SECURITY_MAXIMUM)
        add_compile_options(
            -fstack-protector-all      # Maximum stack protection
            -Wformat-security          # Format string security
            -Wl,-z,defs                # Detect undefined symbols
        )
        add_compile_definitions(FORTIFY_SOURCE=2)
    endif()
endif()

# Sanitizers (for development/testing)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

if(ENABLE_CFI AND NOT MSVC)
    add_compile_options(-fsanitize=cfi)
    add_link_options(-fsanitize=cfi)
endif()

# Build configuration with enhanced optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG_BUILD)
    if(MSVC)
        add_compile_options(/Od /Zi /MDd)
    else()
        add_compile_options(-O0 -g3 -fno-omit-frame-pointer)
    endif()
    
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(RELEASE_BUILD)
    if(MSVC)
        add_compile_options(/O2 /Zi /MD)
    else()
        add_compile_options(-O2 -g1)
    endif()
    
else() # Release builds
    add_compile_definitions(RELEASE_BUILD)
    
    if(MSVC)
        if(OPTIMIZATION_LEVEL STREQUAL "maximum")
            add_compile_options(/Ox /Oi /Ot /Ob2 /GL /favor:INTEL64 /arch:AVX2)
        elseif(OPTIMIZATION_LEVEL STREQUAL "aggressive") 
            add_compile_options(/O2 /Oi /Ot /GL /favor:INTEL64)
        else()
            add_compile_options(/O2)
        endif()
        
        # Link-time optimizations
        if(ENABLE_LTO)
            add_link_options(/LTCG /OPT:REF /OPT:ICF)
        endif()
        
    else() # GCC/Clang
        if(OPTIMIZATION_LEVEL STREQUAL "maximum")
            add_compile_options(-O3 -march=native -mtune=native -flto -ffast-math)
        elseif(OPTIMIZATION_LEVEL STREQUAL "aggressive")
            add_compile_options(-O2 -march=native -flto)
        else()
            add_compile_options(-O2)
        endif()
        
        # Link-time optimizations
        if(ENABLE_LTO)
            add_link_options(-flto -Wl,--gc-sections)
        endif()
    endif()
    
    # Profile-Guided Optimization
    if(ENABLE_PGO)
        if(MSVC)
            add_compile_options(/GL)
            add_link_options(/LTCG:PGI)
        else()
            add_compile_options(-fprofile-generate)
            add_link_options(-fprofile-generate)
        endif()
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
