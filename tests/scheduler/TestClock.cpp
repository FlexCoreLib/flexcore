/*
 * Unit Tests for our virtual clock.
 */

#include <flexcore/scheduler/clock.hpp>
#include <boost/test/unit_test.hpp>


using namespace fc;
namespace chr = std::chrono;

BOOST_AUTO_TEST_SUITE(test_clock)

namespace
{
using master = master_clock<std::centi>;

static constexpr virtual_clock::steady::duration one_tick
		= chr::duration_cast<virtual_clock::duration>(master::duration(1));
}

BOOST_AUTO_TEST_CASE(test_example_uses)
{
	const auto one_tick_ago = virtual_clock::steady::now();
	master::advance();
	const auto now = virtual_clock::steady::now();
	const auto diff = now - one_tick_ago; //the time passed between the two calls to now().

	//since we advanced the clock only once,
	//the time should be equal to the minimal duration of the clock.
	BOOST_CHECK(diff == one_tick);
	BOOST_CHECK(one_tick_ago != now); //sanity check , if one_tick == 0
}

BOOST_AUTO_TEST_CASE(test_advance)
{
	const auto tmp = virtual_clock::steady::now();
	for (int i = 0; i != 1000; ++i)
		master::advance();

	auto now = virtual_clock::steady::now();
	auto diff = now - tmp;
	BOOST_CHECK(diff == one_tick * 1000);
	BOOST_CHECK(tmp != now);
}

BOOST_AUTO_TEST_CASE(test_set_time)
{
	const auto time_null = virtual_clock::system::time_point(
			master::duration::zero());

	master::set_time(time_null);

	const auto tmp = virtual_clock::system::now();

	BOOST_CHECK_EQUAL(tmp.time_since_epoch().count(),
			virtual_clock::duration::zero().count());

	master::advance();
	BOOST_CHECK_EQUAL((virtual_clock::system::now()-tmp).count(),
			one_tick.count());
}

BOOST_AUTO_TEST_CASE(test_time_t_conversion)
{
	const auto now = virtual_clock::system::now();

	const auto c_time = virtual_clock::system::to_time_t(now);
	auto back_converted = virtual_clock::system::from_time_t(c_time);
	//need to cast durations to seconds, since that is the unit of std::time_t
	BOOST_CHECK(chr::time_point_cast<chr::seconds>(now)
			== chr::time_point_cast<chr::seconds>(back_converted));
	BOOST_CHECK_EQUAL(chr::duration_cast<chr::seconds>(now.time_since_epoch()).count(),
			chr::duration_cast<chr::seconds>(back_converted.time_since_epoch()).count());

	for (int i = 0; i != 1000; ++i)
		master::advance();

	const auto now_2 = virtual_clock::system::now();
	BOOST_CHECK(now_2 != now);

	const auto c_time_2 = virtual_clock::system::to_time_t(now_2);
	back_converted = virtual_clock::system::from_time_t(c_time_2);

	BOOST_CHECK(c_time_2 != c_time);

	BOOST_CHECK(chr::time_point_cast<chr::seconds>(now_2)
			== chr::time_point_cast<chr::seconds>(back_converted));
}

BOOST_AUTO_TEST_SUITE_END()
