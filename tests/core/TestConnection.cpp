/*
 * TestConnection.cpp
 *
 *  Created on: Sep 8, 2015
 *      Author: ckielwein
 */

#include "TestConnection.h"

#include "core/connection.hpp"

void TestConnection::TestTrivialExamples() {

	//test trivial connection of two objects
	auto increment = [](int i) -> int {return i+1;};
	auto give_one = [](void) ->int {return 1;};

	assert(increment(1) == 2); // sanity check

	auto one_plus_one = connect(give_one, increment);
	assert(test_connection() == 2);

	//test chained connection
	auto two_plus_one = connect(one_plus_one, increment);
	assert(two_plus_one() == 3);

	// test chain of two anonymous nodes
	auto plus_two = connect([](void) ->int {return 1;},
			[](void) ->int {return 1;});

	assert(plus_two(1) == 3);
}
