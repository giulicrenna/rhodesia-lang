cmake_minimum_required(VERSION 3.16)
project(Rhodesia VERSION 0.1.2 LANGUAGES CXX)

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

# Link statically against libstdc++ / libgcc so the resulting .exe does
# NOT need Strawberry's bin (libgcc_s_seh-1.dll, libstdc++-6.dll) at
# runtime. UCRT (api-ms-win-crt-*) is always present on Windows 10+.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_link_options(-static-libgcc -static-libstdc++)
endif()

# On Windows (MinGW/Strawberry), libwinpthread-1.dll is still linked
# dynamically. Copy the runtime DLLs from the compiler's bin directory
# next to the executable so it can run without Strawberry in PATH.
# (Declared here so the target exists; custom commands attached below.)

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
# On Windows (MSVC / MinGW / Strawberry) we link Winsock 2.
if(WIN32)
    target_link_libraries(rhodesia PRIVATE ws2_32)
endif()

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
# On Windows (MinGW/Strawberry), libwinpthread-1.dll is still linked
# dynamically. Copy the runtime DLLs from the compiler's bin directory
# next to the executable so it can run without Strawberry in PATH.
# ============================================================================
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_program(GCC_PATH gcc.exe PATH_SUFFIXES c/bin REQUIRED)
    get_filename_component(GCC_BIN_DIR "${GCC_PATH}" DIRECTORY)
    set(RUNTIME_DLLS
        libwinpthread-1.dll
        libgcc_s_seh-1.dll
        libstdc++-6.dll
    )
    foreach(dll ${RUNTIME_DLLS})
        if(EXISTS "${GCC_BIN_DIR}/${dll}")
            add_custom_command(TARGET rhodesia POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${GCC_BIN_DIR}/${dll}"
                    "$<TARGET_FILE_DIR:rhodesia>/${dll}"
                COMMENT "Copiando ${dll} junto al ejecutable"
            )
        endif()
    endforeach()
endif()

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
