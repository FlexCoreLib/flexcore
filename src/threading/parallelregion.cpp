/*
 * parallelregion.cpp
 *
 *  Created on: Oct 5, 2015
 *      Author: ckielwein
 */

#include "parallelregion.hpp"

namespace fc
{

bool operator ==(const region_id& lhs, const region_id& rhs)
{
	return true; //todo
}

region_id parallel_region::get_id() const
{
	return id;
}

event_out_port<void> parallel_region::switch_tick()
{
	return ticks.switch_tick();
}

event_out_port<void> parallel_region::work_tick()
{
	return ticks.work_tick();
}

} /* namespace fc */
