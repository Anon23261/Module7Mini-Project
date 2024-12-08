cmake_minimum_required(VERSION 3.15)
project(ghost_os_cpp VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options for different platforms
option(ENABLE_WINDOWS_SUPPORT "Enable Windows-specific features" OFF)
option(ENABLE_ARM_SUPPORT "Enable ARM-specific features" ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Threads REQUIRED)

# Platform-specific configurations
if(WIN32 AND ENABLE_WINDOWS_SUPPORT)
    add_definitions(-DWINDOWS_SUPPORT)
endif()

if(ENABLE_ARM_SUPPORT)
    add_definitions(-DARM_SUPPORT)
endif()

# Main library
add_library(ghost_os_cpp
    src/os_toolkit.cpp
    src/memory_manager.cpp
)

# Include directories
target_include_directories(ghost_os_cpp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(ghost_os_cpp
    PUBLIC
        ${Boost_LIBRARIES}
        Threads::Threads
)

# Platform specific libraries
if(WIN32 AND ENABLE_WINDOWS_SUPPORT)
    target_link_libraries(ghost_os_cpp PUBLIC psapi)
endif()

# Tests
enable_testing()
add_subdirectory(tests)
