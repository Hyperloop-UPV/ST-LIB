#include <gtest/gtest.h>

// Basic test to check that GTest is working
TEST(HelloTest, BasicAssertion) {
    EXPECT_STRNE("hello", "world");
    EXPECT_EQ(7 * 6, 42);
}