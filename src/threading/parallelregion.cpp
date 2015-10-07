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
	std::cout << "region key compare: " << lhs.key << ", " << rhs.key << ", " << (lhs.key == rhs.key) << "\n";
	return lhs.key == rhs.key;
}

region_id parallel_region::get_id() const
{
	return id;
}

parallel_region::parallel_region() :
		ticks(),
		id({"default"}) //todo
{
}

parallel_region::parallel_region(std::string id_) :
		ticks(),
		id({id_})
{
}

event_out_port<void> parallel_region::switch_tick()
{
	std::cout << __FILE__ << __LINE__ << "\n";
	return ticks.switch_tick();
}

event_out_port<void> parallel_region::work_tick()
{
	std::cout << __FILE__ << __LINE__ << "\n";
	return ticks.work_tick();
}

} /* namespace fc */
