#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "histogram/HTree.hpp"

namespace quickstep {

template <typename T>
void test_compare() {
    HypedValue less{ TypedValue{static_cast<T>(0)} };
    HypedValue greater{ TypedValue{static_cast<T>(1)} };

    EXPECT_TRUE(less != greater);
    EXPECT_TRUE(less < greater);
    EXPECT_TRUE(less <= greater);
    EXPECT_TRUE(greater > less);
    EXPECT_TRUE(greater >= less);
    EXPECT_TRUE(!(less == greater));
    EXPECT_TRUE(!(less > greater));
    EXPECT_TRUE(!(less >= greater));
    EXPECT_TRUE(!(greater < less));
    EXPECT_TRUE(!(greater <= less));

    HypedValue h3{greater};
    EXPECT_TRUE(greater == h3);
    EXPECT_TRUE(greater <= h3);
    EXPECT_TRUE(greater >= h3);
    EXPECT_TRUE(!(greater != h3));
    EXPECT_TRUE(!(greater < h3));
    EXPECT_TRUE(!(greater > h3));
}

template <typename T>
void test_width_integer() {
    HypedValue h1{ TypedValue{static_cast<T>(0)} };
    HypedValue h2{ TypedValue{static_cast<T>(1)} };

    EXPECT_TRUE(width(h1, h2) == 2);
    EXPECT_TRUE(width(h1, h1) == 1);
    EXPECT_TRUE(width(h2, h1) == 0);
}

template <typename T>
void test_width_floating_point() {
    HypedValue h1{ TypedValue{static_cast<T>(0)} };
    HypedValue h2{ TypedValue{static_cast<T>(1)} };

    EXPECT_TRUE(width(h1, h2) == 1);
    EXPECT_TRUE(width(h1, h1) == 0);
    EXPECT_TRUE(width(h2, h1) == 0);
}

TEST(HypedValueTest, HypedValueTest_Compare_Int) {
    test_compare<int>();
}

TEST(HypedValueTest, HypedValueTest_Compare_Long) {
    test_compare<long>();
}

TEST(HypedValueTest, HypedValueTest_Compare_Float) {
    test_compare<float>();
}

TEST(HypedValueTest, HypedValueTest_Compare_Double) {
    test_compare<double>();
}

TEST(HypedValueTest, HypedValueTest_Width_Int) {
    test_width_integer<int>();
}

TEST(HypedValueTest, HypedValueTest_Width_Long) {
    test_width_integer<long>();
}

TEST(HypedValueTest, HypedValueTest_Width_Float) {
    test_width_floating_point<float>();
}

TEST(HypedValueTest, HypedValueTest_Width_Double) {
    test_width_floating_point<double>();
}

} // namespace quickstep
