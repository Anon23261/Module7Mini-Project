cmake_minimum_required(VERSION 3.15)
project(cpp_learning_lab VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_SANDBOX "Build the interactive coding sandbox" ON)
option(BUILD_OS_LAB "Build the OS development lab" ON)
option(BUILD_TUTORIALS "Build the tutorial programs" ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
find_package(Curses REQUIRED)

# Add subdirectories
add_subdirectory(sandbox)
add_subdirectory(os_lab)
add_subdirectory(tutorials)
add_subdirectory(examples)

# Testing
enable_testing()
add_subdirectory(tests)
