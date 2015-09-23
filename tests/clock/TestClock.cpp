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
	//since we did nothing but initializing the clocks, they should be equal.
	BOOST_CHECK(virtual_clock::system::now() == virtual_clock::steady::now());


	virtual_clock::master::advance();
	BOOST_CHECK(virtual_clock::system::now() == virtual_clock::steady::now());

	auto one_tick_ago = virtual_clock::steady::now();
	virtual_clock::master::advance();
	auto now = virtual_clock::steady::now();
	auto diff = one_tick_ago - now;
	BOOST_CHECK(diff == virtual_clock::steady::duration::min());
}

BOOST_AUTO_TEST_CASE( test_advance )
{

}
