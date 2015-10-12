#include "cyclecontrol.hpp"

#include <stdexcept>

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
	scheduler.start();
}

void cycle_control::stop()
{
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
	for (auto& task : tasks)
	{ //todo check if task is due
		if (!task.done())  //todo specify error model
			throw out_of_time_exepction();

		task.set_work_to_do(true);
		scheduler.add_task(task);
	}
}

void cycle_control::add_task(periodic_task task)
{
	tasks.push_back(task);
	assert(!tasks.empty());
}

} /* namespace thread */
} /* namespace fc */
