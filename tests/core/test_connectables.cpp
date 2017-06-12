#include <boost/test/unit_test.hpp>
#include <flexcore/core/connectables.hpp>
#include <flexcore/core/connection.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_connectables)

BOOST_AUTO_TEST_CASE(test_arithmetic_and_logical)
{
	auto zero = constant(0);

	BOOST_CHECK_EQUAL((zero >> increment{})(), 1);
	BOOST_CHECK_EQUAL((zero >> decrement{})(), -1);

	BOOST_CHECK_EQUAL((zero >> identity{})(), zero());

	BOOST_CHECK_EQUAL((zero >> add(2))(), 2);

	BOOST_CHECK_EQUAL((zero >> subtract(2))(), -2);

	BOOST_CHECK_EQUAL((zero >> multiply(2))(), 0);
	BOOST_CHECK_EQUAL((zero >> increment{} >> multiply(2))(), 2);

	BOOST_CHECK_EQUAL(([](){ return 4;} >> divide(2))(), 2);

	BOOST_CHECK_EQUAL(([](){ return -1;} >> absolute{})(), 1);
	BOOST_CHECK_EQUAL(([](){ return  1;} >> absolute{})(), 1);
	BOOST_CHECK_EQUAL(([](){ return -1.5f;} >> absolute{})(), 1.5f);
	BOOST_CHECK_EQUAL(([](){ return -1.l;} >> absolute{})(), 1.l);

	BOOST_CHECK_EQUAL(([](){ return 1;} >> negate{})(), -1);

	BOOST_CHECK_EQUAL(([](){ return true;} >> logical_not{})(), false);

	BOOST_CHECK_EQUAL(([](){ return 1;} >> clamp(0,2))(), 1);
	BOOST_CHECK_EQUAL(([](){ return 3;} >> clamp(0,2))(), 2);
	BOOST_CHECK_EQUAL(([](){ return -1;} >> clamp(0,2))(), 0);
}

namespace
{
	template<class T>
	struct mutable_call
	{
		void operator()(T x)
		{
			ref = x;
		}
		T& ref;
	};
}

BOOST_AUTO_TEST_CASE(test_helpers)
{
	auto zero = constant(0);
	BOOST_CHECK_EQUAL(zero(),0);

	auto pi = constant(3.14159);
	BOOST_CHECK_EQUAL(pi(),3.14159);

	//check tee with a functor with const operator()
	int test_value = 0;
	BOOST_CHECK_EQUAL(test_value, 0);
	auto connection = constant(1) >> tee([&test_value](int in){test_value = in;});
	BOOST_CHECK_EQUAL(connection(), 1); //this call triggers the side effect from tee
	BOOST_CHECK_EQUAL(test_value, 1);

	//check tee with a functor with non-const operator()
	auto non_constconnection = constant(2) >> tee(mutable_call<int>{test_value});
	BOOST_CHECK_EQUAL(non_constconnection(), 2);
	BOOST_CHECK_EQUAL(test_value, 2);
}

BOOST_AUTO_TEST_CASE(tee_move_only)
{
	auto src = [] { return std::make_unique<int>(42); };
	std::unique_ptr<int> target{nullptr};
	int tee_target{0};
	auto sink = [&target](auto&& in){ target = std::move(in); };

	auto test_con =	src
			>> tee([&tee_target](const auto& ptr) { tee_target = *ptr; })
			>> sink;

	test_con();

	BOOST_CHECK_EQUAL(tee_target, 42);
	BOOST_CHECK_EQUAL(*target, 42);
}

BOOST_AUTO_TEST_CASE(test_print)
{
	std::ostringstream test_stream;
	auto connection = constant(1) >> print(test_stream);
	connection();

	BOOST_CHECK_EQUAL(test_stream.str(), "1\n");
}

BOOST_AUTO_TEST_CASE(test_constexpr_connectables)
{
	constexpr auto con = increment{} >> decrement{} >> identity{};
	constexpr auto val = con(0);
	static_assert(val == 0, "");
	static_assert(con(1) == 1, "");
}

BOOST_AUTO_TEST_SUITE_END()
