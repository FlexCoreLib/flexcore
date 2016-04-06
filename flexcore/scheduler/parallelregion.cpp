/*
 * parallelregion.cpp
 *
 *  Created on: Oct 5, 2015
 *      Author: ckielwein
 */

#include <scheduler/parallelregion.hpp>

namespace fc
{

bool operator ==(const region_id& lhs, const region_id& rhs)
{
	return lhs.key == rhs.key;
}

region_id parallel_region::get_id() const
{
	return id;
}

parallel_region::parallel_region(std::string id_) :
		ticks(),
		id({id_})
{
}

pure::event_source<void>& parallel_region::switch_tick()
{
	return ticks.switch_tick();
}

pure::event_source<void>& parallel_region::work_tick()
{
	return ticks.work_tick();
}

} /* namespace fc */
