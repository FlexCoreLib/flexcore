/*
 * cyclecontrol.cpp
 *
 *  Created on: Sep 23, 2015
 *      Author: ckielwein
 */

#include "cyclecontrol.hpp"

#include <stdexcept>

#include <iostream>

namespace fc
{
namespace thread
{

using clock = chrono::virtual_clock::master;

struct out_of_time_exepction: std::runtime_error
{
	out_of_time_exepction() :
			std::runtime_error("cyclic task has not finished in time")
	{
	}
};

void cycle_control::start()
{
	//todo
}

void cycle_control::stop()
{
	//todo
	scheduler.stop();
}

void cycle_control::work()
{
	//Todo check pass of time, in particular drift.

	clock::advance();
	run_periodic_tasks();
}

void cycle_control::run_periodic_tasks()
{
	std::cout << "tasks.size() " << tasks.size() << "\n";
	for (auto& task : tasks)
	{ //todo check if task is due
		if (!task.done())  //todo specify error model
			throw out_of_time_exepction();

		task.work_to_do = true;
		scheduler.add_task(task);
	}
}

void cycle_control::add_task(periodic_task task)
{
	tasks.push_back(task);
}

} /* namespace thread */
} /* namespace fc */
