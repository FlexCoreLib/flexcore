/*
 * parallelregion.h
 *
 *  Created on: Oct 5, 2015
 *      Author: ckielwein
 */

#ifndef SRC_THREADING_PARALLELREGION_HPP_
#define SRC_THREADING_PARALLELREGION_HPP_

#include "../ports/pure_ports.hpp"

namespace fc
{

struct region_id
{
	std::string key;
};


bool operator==(const region_id& lhs, const region_id& rhs);

class region_info
{
public:
	virtual region_id get_id() const = 0;
	virtual pure::event_source<void> switch_tick() const = 0;
	virtual pure::event_source<void> work_tick() const = 0;

protected:
	//destructor is not public, as no ownership to regions is given through this.
	~region_info() = default;
};


class tick_controller
{
public:
	tick_controller() = default;

	/// sends void event on the switch tick of the surrounding region
	pure::event_source<void> switch_tick() const { return switch_buffers; }
	/**
	 * \brief  sends void event on the work tick of the surrounding region
	 * connect nodes, that want to be triggered every cycle to this.
	 */
	pure::event_source<void> work_tick() const { return work; }

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

class parallel_region : public region_info
{
public:
	virtual ~parallel_region() = default;
	explicit parallel_region(std::string id = "default");

	region_id get_id() const override;
	pure::event_source<void> switch_tick() const override;
	pure::event_source<void> work_tick() const override;

protected:
	//copy constructor is protected to avoid slicing, since this is meant as base class
	parallel_region(const parallel_region&) = default;
	parallel_region& operator=(const parallel_region&) = default;

public:
	tick_controller ticks;
	region_id id;

};

} /* namespace fc */

#endif /* SRC_THREADING_PARALLELREGION_HPP_ */
