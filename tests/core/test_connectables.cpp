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

BOOST_AUTO_TEST_CASE(test_helpers)
{
	auto zero = constant(0);
	BOOST_CHECK_EQUAL(zero(),0);

	auto pi = constant(3.14159);
	BOOST_CHECK_EQUAL(pi(),3.14159);

	int test_value = 0;
	BOOST_CHECK_EQUAL(test_value, 0);
	auto connection = constant(1) >> tee([&test_value](int in){test_value = in;});
	BOOST_CHECK_EQUAL(connection(), 1); //this call triggers the side effect from tee
	BOOST_CHECK_EQUAL(test_value, 1);
}

BOOST_AUTO_TEST_CASE(test_print)
{
	std::ostringstream test_stream;
	auto connection = constant(1) >> print(test_stream);
	connection();

	BOOST_CHECK_EQUAL(test_stream.str(), "1\n");
}

BOOST_AUTO_TEST_SUITE_END()
