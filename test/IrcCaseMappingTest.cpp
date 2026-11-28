#include "CppUTest/TestHarness.h"
#include "IrcCaseMapping.hpp"

TEST_GROUP(IrcCaseMapping)
{
	void	setup() {}
	void	teardown() {}
};

/**
 * RFC 2812 2.2
 * 
 * "Because of IRC's Scandinavian origin, 
 * the characters '{}|^' are considered to be 
 * the lower case equivalents of the characters '[]\~', respectively."
 */

TEST(IrcCaseMapping, NormalizeConvertsAsciiToLowercase)
{
	STRCMP_EQUAL("nick", IrcCaseMapping::normalize("NICK").c_str());
}

TEST(IrcCaseMapping, NormalizeMapsRfcSpecialChars)
{
	STRCMP_EQUAL("{}|^", IrcCaseMapping::normalize("[]\\~").c_str());
}

TEST(IrcCaseMapping, EqualsComparesCaseInsensitively)
{
	CHECK_TRUE(IrcCaseMapping::equals("Nick[]~", "nick{}^"));
	CHECK_FALSE(IrcCaseMapping::equals("nick1", "nick2"));
}

/**
 * RFC 2812 2.3.1
 * 
 * nickname = ( letter / special ) *8( letter / digit / special / "-" )
 * 
 * letter     = %x41-5A / %x61-7A       ; A-Z / a-z
 * digit      = %x30-39                 ; 0-9
 * special    = %x5B-60 / %x7B-7D       ; "[", "]", "\", "`", "_", "^", "{", "|", "}"
 */

TEST(IrcCaseMapping, isValidNickname)
{
	CHECK_TRUE(IrcCaseMapping::isValidNickname("alice"));
	CHECK_TRUE(IrcCaseMapping::isValidNickname("_alice"));
	CHECK_TRUE(IrcCaseMapping::isValidNickname("alice_"));
	CHECK_TRUE(IrcCaseMapping::isValidNickname("alice`"));
	CHECK_TRUE(IrcCaseMapping::isValidNickname("alice-1"));
	CHECK_TRUE(IrcCaseMapping::isValidNickname("aaaaaa"));
	CHECK_TRUE(IrcCaseMapping::isValidNickname("a23456789"));
	
	CHECK_FALSE(IrcCaseMapping::isValidNickname(""));
	CHECK_FALSE(IrcCaseMapping::isValidNickname("0alice"));
	CHECK_FALSE(IrcCaseMapping::isValidNickname("-alice"));
	CHECK_FALSE(IrcCaseMapping::isValidNickname("a234567890"));

}