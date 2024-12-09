#include <gtest/gtest.h>
#include "../include/os_toolkit.hpp"

TEST(OsToolkitTest, BasicFunctionality) {
    // Add your test cases here
    EXPECT_TRUE(true);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
