/*
 * parallelregion.h
 *
 *  Created on: Oct 5, 2015
 *      Author: ckielwein
 */

#ifndef SRC_THREADING_PARALLELREGION_HPP_
#define SRC_THREADING_PARALLELREGION_HPP_

#include <ports/pure_ports.hpp>

namespace fc
{

/// identifier of a parallel region
struct region_id
{
	std::string key;
};


bool operator==(const region_id& lhs, const region_id& rhs);

/**
 * \brief class providing the interface to cyclic ticks for nodes.
 */
class tick_controller
{
public:
	tick_controller() = default;

	/// sends void event on the switch tick of the surrounding region
	pure::event_source<void>& switch_tick() { return switch_buffers; }
	/**
	 * \brief  sends void event on the work tick of the surrounding region
	 * connect nodes, that want to be triggered every cycle to this.
	 */
	pure::event_source<void>& work_tick() { return work; }

	/**
	 * \brief Buffers in region will be switched when event is received.
	 * connect to scheduler.
	 * expects event with no payload (void).
	 */
	auto in_switch_buffers() { return [this](){return switch_buffers.fire();};}
	/**
	 * \brief work ticks in region will be fired when event is received.
	 * connect to scheduler.
	 * expects event with no payload (void).
	 */
	auto in_work() { return [this](){ return work.fire(); };}

	pure::event_source<void> switch_buffers;
	pure::event_source<void> work;
};

/**
 * \brief Class defining a single parallel region.
 *
 * Provides switch ticks and work ticks for all nodes contained in the region.
 * \see https://gitlab-test.site.x/flexcore/flexcore/wikis/ParallelRegion
 */
class parallel_region
{
public:
	explicit parallel_region(std::string id = "default");

	parallel_region(const parallel_region&) = delete;
	parallel_region(parallel_region&&) = default;

	region_id get_id() const;
	pure::event_source<void>& switch_tick();
	pure::event_source<void>& work_tick();

protected:

public:
	tick_controller ticks;
	region_id id;
};

} /* namespace fc */

#endif /* SRC_THREADING_PARALLELREGION_HPP_ */
