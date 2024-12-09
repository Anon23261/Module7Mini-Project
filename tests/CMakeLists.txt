cmake_minimum_required(VERSION 3.15)
project(SystemsPortfolioTests)

# Enable testing
enable_testing()

# Add test executables
file(GLOB TEST_SOURCES "*.cpp")
foreach(source ${TEST_SOURCES})
    get_filename_component(filename ${source} NAME_WE)
    add_executable(${filename} ${source})
    target_link_libraries(${filename} PRIVATE
        GTest::GTest
        GTest::Main
        ${CMAKE_THREAD_LIBS_INIT}
    )
    add_test(NAME ${filename} COMMAND ${filename})
endforeach()

# Add subdirectories
add_subdirectory(cpp_learning_lab)
add_subdirectory(ghost_os_cpp)
add_subdirectory(ghost_os_toolkit)
add_subdirectory(linux_learning_lab)
