cmake_minimum_required(VERSION 3.15)
project(CppTutorials)

# Add tutorial executables
file(GLOB TUTORIAL_SOURCES "*.cpp")
foreach(source ${TUTORIAL_SOURCES})
    get_filename_component(filename ${source} NAME_WE)
    add_executable(${filename} ${source})
    target_link_libraries(${filename} PRIVATE
        Boost::system
        ${CMAKE_THREAD_LIBS_INIT}
    )
endforeach()
