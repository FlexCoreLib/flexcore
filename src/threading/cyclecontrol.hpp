/*
 * cyclecontrol.hpp
 *
 *  Created on: Sep 23, 2015
 *      Author: ckielwein
 */

#ifndef SRC_THREADING_CYCLECONTROL_HPP_
#define SRC_THREADING_CYCLECONTROL_HPP_

#include "parallelscheduler.hpp"
#include <clock/clock.hpp>

#include <vector>

namespace fc
{
namespace thread
{

struct periodic_task
{
	periodic_task(std::function<void(void)> job,
			chrono::virtual_clock::duration rate) :
				work_to_do(false),
				cycle_rate(rate),
				work(job)
	{

	}

	bool done() const { return !work_to_do; }
	void operator()()
	{
		work_to_do = false;
		work();
	}
	bool work_to_do;
private:
	chrono::virtual_clock::duration cycle_rate;
	/// work to be done every cycle
	std::function<void(void)> work;
};

class cycle_control
{
public:
	/// starts the main loop
	void start();
	/// stops the main loop in all threads
	void stop();

	void work();

	void add_task(periodic_task task);
private:
	void run_periodic_tasks();
	std::vector<periodic_task> tasks;
	parallel_scheduler scheduler;
};

} /* namespace thread */
} /* namespace fc */

#endif /* SRC_THREADING_CYCLECONTROL_HPP_ */
