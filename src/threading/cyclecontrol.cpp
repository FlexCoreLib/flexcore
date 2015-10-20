#include "cyclecontrol.hpp"

#include <stdexcept>
#include <cassert>

namespace fc
{
namespace thread
{

using clock = master_clock<std::centi>;
constexpr wall_clock::steady::duration cycle_control::min_tick_length;
constexpr virtual_clock::steady::duration cycle_control::fast_tick;
constexpr virtual_clock::steady::duration cycle_control::medium_tick;
constexpr virtual_clock::steady::duration cycle_control::slow_tick;

struct out_of_time_exepction: std::runtime_error
{
	out_of_time_exepction() :
			std::runtime_error("cyclic task has not finished in time")
	{
	}
};

bool periodic_task::is_due(virtual_clock::duration time) const
{
	return (time % cycle_rate) == virtual_clock::duration::zero();
}

void cycle_control::start()
{
	scheduler.start();
	keep_working = true;
	// give the main thread some actual work to do (execute infinite main loop)
	main_loop_thread = std::thread([this](){ main_loop();});
}

void cycle_control::stop()
{
	keep_working = false;
	main_loop_control.notify_all(); //in case main loop is currently waiting
	scheduler.stop();
	if (main_loop_thread.joinable())
		main_loop_thread.join();
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
		const auto now = wall_clock::steady::now();
		work();
		main_loop_control.wait_until(
				loop_lock, now + std::chrono::milliseconds(100));
	}
}

cycle_control::~cycle_control()
{
	stop();
}

void cycle_control::run_periodic_tasks()
{
	std::lock_guard<std::mutex> lock(task_queue_mutex);
	for (auto& task : tasks)
	{
		if (task.is_due(virtual_clock::steady::now().time_since_epoch()))
		{
		//	if (!task.done())  //todo specify error model
		//		throw out_of_time_exepction();

			task.set_work_to_do(true);
			task.send_switch_tick();
			scheduler.add_task(task);
		}
	}
}

void cycle_control::add_task(periodic_task task)
{
	std::lock_guard<std::mutex> lock(task_queue_mutex);
	tasks.push_back(task);
	assert(!tasks.empty());
}

} /* namespace thread */
} /* namespace fc */
