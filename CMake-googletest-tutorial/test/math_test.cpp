#include <gtest/gtest.h>
#include "../src/math.h"

TEST(MathAddTest, EasyAddition) {
    EXPECT_EQ(2, math::add(1, 1));
    EXPECT_EQ(0, math::add(0, 0));
}

TEST(MathSubTest, EasySubtraction) {
    EXPECT_EQ(1, math::sub(2, 1));
    EXPECT_EQ(-1, math::sub(1, 2));
}

TEST(MathMulTest, EasyMultiplication) {
    EXPECT_EQ(4, math::mul(2, 2));
    EXPECT_EQ(0, math::mul(0, 100));
}
