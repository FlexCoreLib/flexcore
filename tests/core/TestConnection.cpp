#include "core/connection.hpp"

// boost
#include <boost/test/unit_test.hpp>

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

// test cases for different pairs of parameter and result types
BOOST_AUTO_TEST_CASE(parameter_result_pairs)
{
	// this variable is captures by lambdas
	// to check if lambdas without return values work.
	int capture_ref = 0;

	auto write_param = [&](int i){ capture_ref = i; };
	auto increment = [](int i) -> int {return i+1;};

	// this connection takes a parameter and returns void
	// An int is transmitted as payload between source and sink.
	auto write_increment = connect(increment, write_param);
	write_increment(0);
	BOOST_CHECK(capture_ref == 1);

	auto do_nothing = [](){};
	auto set_two = [&](){ capture_ref = 2; };
	// This connection takes no parameter, and returns void
	// There is also no payload between source and sink.
	auto set_one_connection = connect(do_nothing, set_two);
	set_one_connection();
	BOOST_CHECK(capture_ref == 2);

	// This connection takes no paramter and returns and int.
	// An int is transmitted as payload between source and sink.
	auto give_one = [](){return 1;};
	BOOST_CHECK(connect(give_one, increment)() == 2);

	// This connection takes no paramter and returns and int.
	// There is also no payload between source and sink.
	BOOST_CHECK(connect(do_nothing, give_one)() == 1);

	// This connection takes no paramter and returns and void.
	// There is also no payload between source and sink.
	auto write_nine = [&](){ capture_ref = 9; };
	connect(do_nothing, write_nine)();
	BOOST_CHECK(capture_ref == 9);

}
