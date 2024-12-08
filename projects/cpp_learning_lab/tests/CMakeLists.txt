cmake_minimum_required(VERSION 3.15)
project(CppTests)

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
