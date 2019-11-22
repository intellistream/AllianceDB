//
// Created by Shuhao Zhang on 22/11/19.
//

#include "gtest/gtest.h"

int add(int a, int b) {
    return a + b;
}

TEST(test1, c1) {
    EXPECT_EQ(3, add(1, 2));
}

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}