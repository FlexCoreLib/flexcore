/*
 * benchmarkfunctions.h
 *
 *  Created on: Feb 7, 2017
 *      Author: ckielwein
 */

#ifndef BENCHMARKS_BENCHMARKFUNCTIONS_H_
#define BENCHMARKS_BENCHMARKFUNCTIONS_H_

#include <flexcore/pure/pure_ports.hpp>
#include <flexcore/extended/nodes/terminal.hpp>
#include <flexcore/ports.hpp>

#include "../tests/nodes/owning_node.hpp"

float identity_function(float in);

struct identity_node
{

	identity_node();

	fc::tests::owning_node owner;
	fc::state_terminal<float>& internal;
};

struct base_class
{
	virtual float foo(float in) const = 0;
	virtual ~base_class() = default;
};

std::unique_ptr<base_class> make_inherited();

struct inherited : base_class
{
	float foo(float in) const override;
};


#endif /* BENCHMARKS_BENCHMARKFUNCTIONS_H_ */
