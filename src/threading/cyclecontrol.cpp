#include "cyclecontrol.hpp"

#include <stdexcept>

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

/// returns true if cycle _rate of task matches time and work of task is due.
bool periodic_task::is_due(virtual_clock::steady::time_point now) const
{
	auto time = now .time_since_epoch();
	return (time % interval) == virtual_clock::duration::zero();
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
				loop_lock, now + min_tick_length);
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
		if (task.is_due(virtual_clock::steady::now()))
		{
			if (!task.done())  //todo specify error model
			{
				std::exception_ptr ep = std::make_exception_ptr(out_of_time_exepction());
				std::lock_guard<std::mutex> lock(task_exception_mutex);
				task_exceptions.push_back(ep);
				keep_working=false;
				return;
			}

			task.set_work_to_do(true);
			task.send_switch_tick();
			scheduler.add_task([&task] { task(); });
		}
	}
}

void cycle_control::add_task(periodic_task task)
{
	std::lock_guard<std::mutex> lock(task_queue_mutex);
	tasks.push_back(std::move(task));
	assert(!tasks.empty());
}

std::exception_ptr cycle_control::last_exception()
{
	std::lock_guard<std::mutex> lock(task_exception_mutex);
	if(task_exceptions.empty())
		return nullptr;
	std::exception_ptr except = task_exceptions.back();
	task_exceptions.pop_back();
	return except;
}

} /* namespace thread */
} /* namespace fc */
