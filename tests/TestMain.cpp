#include "gtest\gtest.h"
#include "TestPrinter.hpp"

using namespace testing;

int main(int argc, char **argv)
{
	InitGoogleTest(&argc, argv);
	UnitTest& unit_test = *UnitTest::GetInstance();
	TestEventListeners& listeners = unit_test.listeners();
	listeners.Append(new TestPrinter());
	auto ret = RUN_ALL_TESTS();
	return ret;
}
