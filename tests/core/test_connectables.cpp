#include <boost/test/unit_test.hpp>
#include <flexcore/core/connectables.hpp>
#include <flexcore/core/connection.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_connectables)

BOOST_AUTO_TEST_CASE(test_arithmetic_and_logical)
{
	auto zero = [](){ return 0; };

	BOOST_CHECK_EQUAL((zero >> increment)(), 1);
	BOOST_CHECK_EQUAL((zero >> decrement)(), -1);

	BOOST_CHECK_EQUAL((zero >> identity)(), zero());

	BOOST_CHECK_EQUAL((zero >> add_to(2))(), 2);

	BOOST_CHECK_EQUAL((zero >> subtract_value(2))(), -2);

	BOOST_CHECK_EQUAL((zero >> multiply_with(2))(), 0);
	BOOST_CHECK_EQUAL((zero >> increment >> multiply_with(2))(), 2);

	BOOST_CHECK_EQUAL(([](){ return 4;} >> divide_by(2))(), 2);

	BOOST_CHECK_EQUAL(([](){ return -1;} >> absolute)(), 1);
	BOOST_CHECK_EQUAL(([](){ return -1.f;} >> absolute)(), 1.f);
	BOOST_CHECK_EQUAL(([](){ return -1.l;} >> absolute)(), 1.l);

	BOOST_CHECK_EQUAL(([](){ return 1;} >> negate)(), -1);

	BOOST_CHECK_EQUAL(([](){ return true;} >> logical_not)(), false);

	BOOST_CHECK_EQUAL(([](){ return 1;} >> clamp(0,2))(), 1);
	BOOST_CHECK_EQUAL(([](){ return 3;} >> clamp(0,2))(), 2);
	BOOST_CHECK_EQUAL(([](){ return -1;} >> clamp(0,2))(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
