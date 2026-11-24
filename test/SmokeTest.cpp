#include <string>
#include "CppUTest/TestHarness.h"

TEST_GROUP(Smoke)
{
	void	setup() {}
	void	teardown() {}
};

TEST(Smoke, CppUTestRunsAndSupportsStdString)
{
	int	value = 1 + 1;
	LONGS_EQUAL(2, value);

	std::string	msg = "ft_irc";
	STRCMP_EQUAL("ft_irc", msg.c_str());
	CHECK_TRUE(msg == "ft_irc");
}