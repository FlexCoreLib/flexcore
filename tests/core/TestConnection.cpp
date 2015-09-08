/*
 * TestConnection.cpp
 *
 *  Created on: Sep 8, 2015
 *      Author: ckielwein
 */

#include "TestConnection.h"

#include "src/core/connection.hpp"

void TestConnection::TestTrivialExamples() {

	auto increment = [](int i) -> int { return i+1; };
	auto give_one = [](void) ->int { return 1; };


	auto test_connection = connect(give_one, increment);
}
