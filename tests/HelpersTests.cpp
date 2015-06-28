#include "gtest/gtest.h"
#include "Helpers.hpp"
#include "MakeArray.hpp"

TEST(HelpersTest, isDominatedSingleDimension)
{
	EXPECT_FALSE(isDominated(make_array(5), make_array(3)));
	EXPECT_TRUE(isDominated(make_array(2), make_array(3)));
	EXPECT_FALSE(isDominated(make_array(3), make_array(3)));
}

TEST(HelpersTest, isDominatedTwoDimensions)
{
	EXPECT_FALSE(isDominated(make_array(5, 3), make_array(2, 1)));
	EXPECT_FALSE(isDominated(make_array(5, 3), make_array(5, 2)));
	EXPECT_FALSE(isDominated(make_array(5, 3), make_array(4, 4)));
	EXPECT_FALSE(isDominated(make_array(5, 3), make_array(5, 3)));
	EXPECT_TRUE(isDominated(make_array(5, 3), make_array(6, 3)));
	EXPECT_TRUE(isDominated(make_array(5, 3), make_array(5, 4)));
	EXPECT_TRUE(isDominated(make_array(5, 3), make_array(6, 4)));
}