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
	, internal(owner.make_child_named<fc::event_terminal<float>>("bla"))
{
}


float identity_node::foo(float in)
{
	using fc::operator>>;

	float out = 0;
	auto out_writer = [&](float x){ out = x; };
	internal.out() >> out_writer;
	internal.in()(in);
	return out;
}
