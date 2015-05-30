#include "gtest\gtest.h"
#include "TestPrinter.hpp"

using namespace testing;

TEST(TestSystemIsWorking, TestSystemIsWorking)
{
	EXPECT_TRUE(true);
}

int main(int argc, char **argv)
{
	InitGoogleTest(&argc, argv);
	UnitTest& unit_test = *UnitTest::GetInstance();
	TestEventListeners& listeners = unit_test.listeners();
	listeners.Append(new TestPrinter());
	return RUN_ALL_TESTS();
}
