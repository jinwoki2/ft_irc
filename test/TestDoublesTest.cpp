#include "CppUTest/TestHarness.h"
#include "ISender.hpp"
#include "SpySender.hpp"
#include "IClock.hpp"
#include "StubClock.hpp"
#include "IProtocol.hpp"

TEST_GROUP(TestDoubles)
{
	void	setup() {}
	void	teardown() {}
};

TEST(TestDoubles, SpySenderRecordsSendCalls)
{
	SpySender	spy;
	ISender*	sender = &spy;

	sender->send(1, "PING :server\r\n");

	UNSIGNED_LONGS_EQUAL(1, spy.sent.size());
	LONGS_EQUAL(1, spy.sent[0].fd);
	STRCMP_EQUAL("PING :server\r\n", spy.sent[0].data.c_str());
}

TEST(TestDoubles, SpySenderRecordsCloseCalls)
{
	SpySender	spy;
	ISender*	sender = &spy;

	sender->close(4);
	UNSIGNED_LONGS_EQUAL(1, spy.closed.size());
	LONGS_EQUAL(4, spy.closed[0]);
}

TEST(TestDoubles, StubClockReturnsFixedTime)
{
	StubClock	clock(1700000000, "2023-11-14");
	IClock*		iface = &clock;

	LONGS_EQUAL(1700000000, iface->now());
}

TEST(TestDoubles, StubClockReturnsFixedFormattedTime)
{
	StubClock	clock(1700000000, "2023-11-14");
	IClock*		iface = &clock;

	STRCMP_EQUAL("2023-11-14", iface->nowFormatted().c_str());
}
