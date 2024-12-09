cmake_minimum_required(VERSION 3.15)
project(OsLab)

# Add OS lab executables
add_executable(bootloader bootloader.cpp)
add_executable(kernel kernel.cpp)
add_executable(memory_manager memory_manager.cpp)

# Link dependencies
target_link_libraries(bootloader PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(kernel PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(memory_manager PRIVATE ${CMAKE_THREAD_LIBS_INIT})
