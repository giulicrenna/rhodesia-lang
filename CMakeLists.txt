cmake_minimum_required(VERSION 3.16)
project(Rhodesia VERSION 0.1.0 LANGUAGES CXX)

# ============================================================================
# Compiler Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release for single-config generators if no build type is provided.
# This avoids accidentally benchmarking/debugging an unoptimized binary.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    message(STATUS "No CMAKE_BUILD_TYPE specified, defaulting to Release")
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -march=native)
    endif()
endif()

# ============================================================================
# Dependencies
# ============================================================================

# Find Eigen3
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
if(Eigen3_FOUND)
    message(STATUS "Found Eigen3: ${EIGEN3_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "Eigen3 not found. Install with: sudo apt install libeigen3-dev")
endif()

# ============================================================================
# Main Executable
# ============================================================================
add_executable(rhodesia
    src/main.cpp
)

# Include directories
target_include_directories(rhodesia PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(rhodesia PRIVATE
    Eigen3::Eigen
)

# BSD sockets are part of glibc on Linux; no extra library needed.
# On some exotic targets (e.g. MinGW) you would add -lws2_32 here.

# ============================================================================
# Installation
# ============================================================================
install(TARGETS rhodesia
    RUNTIME DESTINATION bin
)

# ============================================================================
# CPack for packaging
# ============================================================================
set(CPACK_PACKAGE_NAME "rhodesia")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A strongly-typed language for Data Science")
include(CPack)

# ============================================================================
# Status Message
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  Rhodesia Configuration Summary")
message(STATUS "========================================")
message(STATUS "  Version:      ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "  Eigen3:       Found")
message(STATUS "========================================")
message(STATUS "")
