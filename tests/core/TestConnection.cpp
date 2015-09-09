#include "TestConnection.h"

#include "core/connection.hpp"

// boost
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(foo)
{

	//test trivial connection of two objects
	auto increment = [](int i) -> int {return i+1;};
	auto give_one = [](void) ->int {return 1;};

	BOOST_CHECK(increment(1) == 2); // sanity check

	auto one_plus_one = connect(give_one, increment);
	BOOST_CHECK(one_plus_one() == 2);

	//test chained connection
	auto two_plus_one = connect(one_plus_one, increment);
	BOOST_CHECK(two_plus_one() == 3);

	// test chain of two anonymous nodes
	auto plus_two = connect
		(
			[](int i) ->int {return i+1;},
			[](int i) ->int {return i+1;}
		);

	BOOST_CHECK(plus_two(1) == 3);
}



