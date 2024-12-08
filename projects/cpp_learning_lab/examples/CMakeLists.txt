cmake_minimum_required(VERSION 3.15)
project(CppExamples)

# Add example executables
file(GLOB EXAMPLE_SOURCES "*.cpp")
foreach(source ${EXAMPLE_SOURCES})
    get_filename_component(filename ${source} NAME_WE)
    add_executable(${filename} ${source})
    target_link_libraries(${filename} PRIVATE
        Boost::system
        ${CMAKE_THREAD_LIBS_INIT}
    )
endforeach()
