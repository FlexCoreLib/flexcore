/*
 * parallelregion.cpp
 *
 *  Created on: Oct 5, 2015
 *      Author: ckielwein
 */

#include <flexcore/scheduler/parallelregion.hpp>
#include <flexcore/scheduler/cyclecontrol.hpp>

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

virtual_clock::steady::duration parallel_region::get_duration() const
{
	return tick_duration;
}

parallel_region::parallel_region(std::string id_, virtual_clock::steady::duration tick_rate) :
		ticks(),
		id({std::move(id_)}),
		tick_duration(tick_rate)
{
	static_assert(thread::cycle_control::slow_tick == std::chrono::seconds(1),
			"Slow tick is not 1s, the default constructor parameter of parallel_region needs adaption");
}

std::shared_ptr<parallel_region>
parallel_region::new_region(std::string name, virtual_clock::steady::duration tick_rate) const
{
	return std::make_shared<parallel_region>(std::move(name), tick_rate);
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
