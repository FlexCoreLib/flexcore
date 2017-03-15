/*
 * benchmarkfunctions.cpp
 *
 *  Created on: Feb 7, 2017
 *      Author: ckielwein
 */

#include "benchmarkfunctions.h"

float identity_function(const float in)
{
	return in;
}

float inherited::foo(float in) const
{
	return in;
}

identity_node::identity_node()
	: owner()
	, internal(owner.make_child_named<fc::state_terminal<float>>("bla"))
{
}

std::unique_ptr<base_class> make_inherited()
{
	return std::make_unique<inherited>();
}