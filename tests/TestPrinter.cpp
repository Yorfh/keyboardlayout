#include <windows.h>
#include "TestPrinter.hpp"

using namespace testing;

void outDebugStringA(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int len = _vscprintf(format, args) + 1;
	char str[16*1024];
	vsprintf_s(str, format, args);
	OutputDebugStringA(str);
}

// Called after all test activities have ended.
void TestPrinter::OnTestProgramEnd(const UnitTest& unit_test)
{
	outDebugStringA("TEST %s\n", unit_test.Passed() ? "PASSED" : "FAILED");
}

// Called before a test starts.
void TestPrinter::OnTestStart(const TestInfo& test_info) 
{
	outDebugStringA(
		"*** Test %s.%s starting.\n",
		test_info.test_case_name(),
		test_info.name());
}

// Called after a failed assertion or a SUCCEED() invocation.
void TestPrinter::OnTestPartResult(const TestPartResult& test_part_result) 
{
	outDebugStringA(
		"%s in\n%s(%d):\n%s\n",
		test_part_result.failed() ? "*** Failure" : "Success",
		test_part_result.file_name(),
		test_part_result.line_number(),
		test_part_result.summary());
}

// Called after a test ends.
void TestPrinter::OnTestEnd(const TestInfo& test_info) 
{
	outDebugStringA(
		"*** Test %s.%s ending.\n",
		test_info.test_case_name(),
		test_info.name());
}