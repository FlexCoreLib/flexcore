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
	bool done() { return work_to_do; }
	void operator()()
	{
		work_to_do = false;
	}
	bool work_to_do;
private:
	chrono::virtual_clock::duration cycle_rate;
};

class cycle_control
{
	/// starts the main loop
	void start();
	/// stops the main loop in all threads
	void stop();

	void work();
private:
	void run_periodic_tasks();
	std::vector<periodic_task> tasks;
	parallel_scheduler scheduler;
};

} /* namespace thread */
} /* namespace fc */

#endif /* SRC_THREADING_CYCLECONTROL_HPP_ */
