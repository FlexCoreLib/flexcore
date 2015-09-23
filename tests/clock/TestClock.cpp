/*
 * Unit Tests for our virtual clock.
 */

// boost
#include <boost/test/unit_test.hpp>

#include <clock/clock.hpp>

using namespace fc;
using namespace chrono;

BOOST_AUTO_TEST_CASE(test_example_uses)
{
	virtual_clock::master test;
	test.set_time(virtual_clock::time_point::min());
}
