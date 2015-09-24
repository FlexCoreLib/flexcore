/*
 * Unit Tests for our virtual clock.
 */

// boost
#include <boost/test/unit_test.hpp>

#include <clock/clock.hpp>

#include <iostream>

using namespace fc;
using namespace chrono;

BOOST_AUTO_TEST_CASE(test_example_uses)
{
	auto one_tick_ago = virtual_clock::steady::now();
	virtual_clock::master::advance();
	auto now = virtual_clock::steady::now();
	auto diff = one_tick_ago - now; //the time passed between the two calls to now().

	//since we advanced the clock only once,
	//the time should be equal to the minimal duration of the clock.
	BOOST_CHECK(diff == virtual_clock::steady::duration::min());
}

BOOST_AUTO_TEST_CASE(test_advance)
{
	auto tmp = virtual_clock::steady::now();
	for (int i = 0; i != 1000; ++i)
		virtual_clock::master::advance();

	auto now = virtual_clock::steady::now();
	auto diff = tmp - now;
	BOOST_CHECK(diff == virtual_clock::steady::duration::min() * 1000);
}
