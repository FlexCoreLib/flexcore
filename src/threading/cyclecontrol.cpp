#include "cyclecontrol.hpp"

#include <stdexcept>

namespace fc
{
namespace thread
{

using clock = chrono::virtual_clock::master;
constexpr chrono::wall_clock::steady::duration cycle_control::min_tick_length;

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
	keep_working = true;
}

void cycle_control::stop()
{
	scheduler.stop();
	keep_working = false;
}

void cycle_control::work()
{
	clock::advance();
	run_periodic_tasks();
}

void cycle_control::main_loop()
{
	while(keep_working)
	{
		std::unique_lock<std::mutex> loop_lock(main_loop_mutex);
		const auto now = chrono::wall_clock::steady::now();
		work();
		main_loop_control.wait_until(loop_lock, now + cycle_control::min_tick_length);
	}
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
