cmake_minimum_required(VERSION 3.15)
project(SystemsPortfolio VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# Find required packages
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/projects/cpp_learning_lab/include
    ${PROJECT_SOURCE_DIR}/projects/ghost_os_cpp/include
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)

# Add subdirectories
add_subdirectory(projects/cpp_learning_lab)
add_subdirectory(projects/ghost_os_cpp)

# Vulnerability Scanner executable
add_executable(vulnerability_scanner 
    projects/vulnerability_scanner.cpp
)
target_link_libraries(vulnerability_scanner
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
    ${SQLite3_LIBRARIES}
    pthread
)

# Testing
enable_testing()
add_subdirectory(tests)
