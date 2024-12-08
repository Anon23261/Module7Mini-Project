cmake_minimum_required(VERSION 3.15)
project(CppSandbox)

# Add executables
file(GLOB SANDBOX_SOURCES "*.cpp")
foreach(source ${SANDBOX_SOURCES})
    get_filename_component(filename ${source} NAME_WE)
    add_executable(${filename} ${source})
    target_link_libraries(${filename} PRIVATE
        Boost::system
        Boost::filesystem
        ${CMAKE_THREAD_LIBS_INIT}
    )
endforeach()
