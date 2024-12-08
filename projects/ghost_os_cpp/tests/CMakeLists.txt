find_package(GTest REQUIRED)

add_executable(os_toolkit_test
    os_toolkit_test.cpp
    memory_manager_test.cpp
)

target_link_libraries(os_toolkit_test
    PRIVATE
        ghost_os_cpp
        GTest::GTest
        GTest::Main
)

add_test(NAME os_toolkit_test COMMAND os_toolkit_test)
