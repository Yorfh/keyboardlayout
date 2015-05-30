#pragma once
#include "gtest/gtest.h"

class TestPrinter : public testing::EmptyTestEventListener 
{
public:
	virtual void OnTestProgramEnd(const testing::UnitTest& unit_test);
	virtual void OnTestStart(const testing::TestInfo& test_info);
	virtual void OnTestPartResult(const testing::TestPartResult& test_part_result);
	virtual void OnTestEnd(const testing::TestInfo& test_info);
};