#include "core/connection.hpp"

// boost
#include <boost/test/unit_test.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_connection)

/**
 * Example test cases for core connections.
 * These this test shows how connections are used in general.
 * Tests for special cases are below.
 */
BOOST_AUTO_TEST_CASE(examples)
{
	//test trivial connection of two objects
	auto increment = [](int i) -> int {return i+1;}; //just adds 1 to parameter
	auto give_one = [](void) ->int {return 1;}; //simply return 1, no parameter

	// this connection has no parameter and returns and int.
	auto one_plus_one = connect(give_one, increment);
	BOOST_CHECK(one_plus_one() == 2);

	//test chained connection,
	//take connection from above and add a new connectable as a sink.
	auto two_plus_one = connect(one_plus_one, increment);
	BOOST_CHECK(two_plus_one() == 3);

	//connections can have both, parameters and return values:
	{
		// connection of two anonymous nodes, takes and returns an int.
		auto plus_two = connect
			(
				[](int i) ->int {return i+1;},
				[](int i) ->int {return i+1;}
			);
		BOOST_CHECK(plus_two(1) == 3);
	}
	//this is completely equivalent to:
	{
		auto plus_two = connect(increment, increment);
		BOOST_CHECK(plus_two(1) == 3);
	}
}

BOOST_AUTO_TEST_CASE(stream_operator_example)
{
	auto make_four = [](){return 1;} // one
			>> [](int i){return ++i;} // plus one
			>> [](int i){return i+2;}; // plus two
	BOOST_CHECK_EQUAL(make_four(), 4); // is four
}

// test cases for different pairs of parameter and result types
BOOST_AUTO_TEST_CASE(parameter_result_pairs)
{
	// this variable is captures by lambdas
	// to check if lambdas without return values work.
	int capture_ref = 0;

	//named differnt sources and sinks to make tests more readable
	auto write_param = [&](int i){ capture_ref = i; };
	auto increment = [](int i) -> int {return i+1;};
	auto give_one = [](){return 1;};
	auto give_three = [](){return 3;};
	auto do_nothing = [](){};
	auto ignore_in = [](int){};
	auto increment_ref = [&](){ capture_ref++; };

	// param int,  payload int,  result int
	auto plus_two = connect(increment, increment);
	BOOST_CHECK_EQUAL(plus_two(1), 3);

	// param int,  payload int,  result void
	// this connection takes a parameter and returns void
	// An int is transmitted as payload between source and sink.
	auto write_increment = connect(increment, write_param);
	write_increment(0);
	BOOST_CHECK_EQUAL(capture_ref,1);

	// param int,  payload void, result int
	auto ignore_input_return1 = connect(ignore_in, give_one);
	BOOST_CHECK_EQUAL(ignore_input_return1(99), 1);

	// param int,  payload void, result void
	connect(ignore_in, increment_ref)(99);
	BOOST_CHECK_EQUAL(capture_ref, 2);

	// param void, payload int,  result int
	// This connection takes no paramter and returns an int.
	// An int is transmitted as payload between source and sink.
	BOOST_CHECK_EQUAL(connect(give_one, increment)(), 2);

	// param void, payload int,  result void
	connect(give_three, write_param)();
	BOOST_CHECK_EQUAL(capture_ref, 3);

	// param void, payload void, result int
	// This connection takes no paramter and returns and int.
	// There is also no payload between source and sink.
	BOOST_CHECK_EQUAL(connect(do_nothing, give_one)(),1);

	// param void, payload void, result void
	// This connection takes no parameter, and returns void
	// There is also no payload between source and sink.
	connect(do_nothing, increment_ref)();
	BOOST_CHECK_EQUAL(capture_ref, 4);
}

BOOST_AUTO_TEST_CASE(test_move_only_token)
{
	//test if tokens, which cannot be copy constructed but move constructed
	//are properly transmitted.
	// also serves as a test if we don't do unnecessary copies
	struct move_only
	{
		std::unique_ptr<int> val;
	};

	auto source = [](){ return move_only{ std::make_unique<int>(1)};};
	auto increment = [](move_only in) { *(in.val) = *(in.val)+1; return in; };

	auto connect = source >> increment;
	auto result = connect();
	BOOST_CHECK_EQUAL(*(result.val), 2);
}

BOOST_AUTO_TEST_CASE(test_polymorphic_lambda)
{
	auto give_one = [](void) ->int {return 1;}; //simply return 1, no parameter
	auto poly_increment = [](auto i){ return ++i; };

	auto one_plus_one = connect(give_one, poly_increment);
	BOOST_CHECK(one_plus_one() == 2);
}
/**
 * Confirm that connecting connectables
 * does not depend on any particular order.
 */
BOOST_AUTO_TEST_CASE( associativity )
{
	// this variable is captures by lambdas
	int tee_ref = 0;
	int sink_ref = 0;
	int source = 1;

	//named differnt sources and sinks to make tests more readable
	auto give_source = [&]() {return source; };
	auto increment = [](int i) -> int { return i + 1; };
	auto tee = [&](int i) { tee_ref = i; return i; };
	auto write_param = [&](int i){ sink_ref = i; };

	auto a = give_source;
	auto b = increment;
	auto c = tee;
	auto d = write_param;

	source = 10;
	(a >> b >> c >> d)();
	BOOST_CHECK_EQUAL(tee_ref, 11);
	BOOST_CHECK_EQUAL(sink_ref, 11);

	source = 20;
	((a >> b) >> (c >> d))();
	BOOST_CHECK_EQUAL(tee_ref, 21);
	BOOST_CHECK_EQUAL(sink_ref, 21);

	source = 50;
	((a >> (b >> c)) >> d)();
	BOOST_CHECK_EQUAL(tee_ref, 51);
	BOOST_CHECK_EQUAL(sink_ref, 51);

	source = 60;
	(a >> ((b >> c) >> d))();
	BOOST_CHECK_EQUAL(tee_ref, 61);
	BOOST_CHECK_EQUAL(sink_ref, 61);
}

BOOST_AUTO_TEST_CASE( result_of_connection)
{
	auto source = [] { return 1; };
	auto sink = [] (int x) { return static_cast<float>(x); };
	auto conn = source >> sink;
	static_assert(is_instantiation_of<connection, decltype(conn)>{},
	              "operator >> for connectable should return connection");
	static_assert(std::is_same<result_of_t<decltype(conn)>, decltype(conn())>{},
	              "result_of connection should be the same as return type of conn()");
}

BOOST_AUTO_TEST_SUITE_END()
