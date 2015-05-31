#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Keyboard.hpp" 

using namespace testing;

TEST(KeyboardTests, Randomize)
{
	auto k = Keyboard();
	using KeyArray = decltype(Keyboard::m_keys);
	std::mt19937 randomGenerator;

	auto testKeyboard = [&k, &randomGenerator](KeyArray& a)
	{
		for (int i = 0;i < 30; i++)
		{
			k.randomize(randomGenerator);
			if (k.m_keys == a)
				break;
		}
		EXPECT_THAT(k.m_keys, ElementsAreArray(a.begin(), a.end()));
	};

	testKeyboard(KeyArray{ 0, 1, 2 });
	testKeyboard(KeyArray{ 0, 2, 1 });
	testKeyboard(KeyArray{ 1, 0, 2 });
	testKeyboard(KeyArray{ 1, 2, 0 });
	testKeyboard(KeyArray{ 2, 0, 1 });
	testKeyboard(KeyArray{ 2, 1, 0 });
}