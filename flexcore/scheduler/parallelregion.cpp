/*
 * parallelregion.cpp
 *
 *  Created on: Oct 5, 2015
 *      Author: ckielwein
 */

#include <flexcore/scheduler/parallelregion.hpp>

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
		id({std::move(id_)})
{
}

std::shared_ptr<parallel_region>
parallel_region::new_region(std::string name, virtual_clock::steady::duration /* tick_rate */) const
{
	// TODO: check if there is any good way to use tick_rate here.
	return std::make_shared<parallel_region>(std::move(name));
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
