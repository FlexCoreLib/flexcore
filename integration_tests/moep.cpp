/*
 * moep.cpp
 *
 *  Created on: Mar 11, 2016
 *      Author: ckielwein
 */

#include "moep.h"
#include <range/actions.hpp>
#include <range/algorithm.hpp>
#include <core/connection.hpp>

using namespace fc;

int foo(std::vector<int>& in)
{
	auto con = actions::filter([](int i){ return i < 0;})
			>> actions::map([](int i){ return i*2;})
			>> sum(0);

	return con(in);
}
