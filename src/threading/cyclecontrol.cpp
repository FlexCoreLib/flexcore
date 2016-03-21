#include "cyclecontrol.hpp"

#include <stdexcept>
#include <algorithm>

namespace fc
{
namespace thread
{

using clock = master_clock<std::centi>;
constexpr wall_clock::steady::duration cycle_control::min_tick_length;
constexpr virtual_clock::steady::duration cycle_control::fast_tick;
constexpr virtual_clock::steady::duration cycle_control::medium_tick;
constexpr virtual_clock::steady::duration cycle_control::slow_tick;

void cycle_control::start()
{
	keep_working = true;
	running = true;
	// give the main thread some actual work to do (execute infinite main loop)
	main_loop_thread = std::thread([this](){ main_loop();});
}

void cycle_control::stop()
{
	{
		std::lock_guard<std::mutex> lock(main_loop_mutex);
		keep_working = false;
		main_loop_control.notify_one(); //in case main loop is currently waiting
	}
	scheduler.stop();
	if (main_loop_thread.joinable())
		main_loop_thread.join();
	running = false;
}

void cycle_control::work()
{
	auto now = virtual_clock::steady::now();
	auto run_if_due = [this, now](auto tick_rate, auto& task_vector)
	{
		if (now.time_since_epoch() % tick_rate == virtual_clock::duration::zero())
			if (!run_periodic_tasks(task_vector))
				return false;
		return true;
	};
	if (!run_if_due(slow_tick, tasks_slow)) return;
	if (!run_if_due(medium_tick, tasks_medium)) return;
	if (!run_if_due(fast_tick, tasks_fast)) return;
	clock::advance();
}

void cycle_control::main_loop()
{
	// the mutex needs to be held to check the condition; wait_until releases the lock while waiting.
	std::unique_lock<std::mutex> loop_lock(main_loop_mutex);
	while(keep_working)
	{
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

bool cycle_control::run_periodic_tasks(std::vector<periodic_task>& tasks)
{
	//todo specify error model
	if (any_of(begin(tasks), end(tasks), [](auto& task) { return !task.done(); }))
	{
		auto ep = std::make_exception_ptr(out_of_time_exception());
		std::lock_guard<std::mutex> lock(task_exception_mutex);
		task_exceptions.push_back(ep);
		keep_working = false;
		return false;
	}

	for (auto& task : tasks)
	{
		task.set_work_to_do(true);
		task.send_switch_tick();
		scheduler.add_task([&task] { task(); });
	}
	return true;
}

void cycle_control::add_task(periodic_task task, virtual_clock::duration tick_rate)
{
	if (running)
		throw std::runtime_error{"Worker threads are already running"};

	if (tick_rate == slow_tick)
		tasks_slow.emplace_back(std::move(task));
	else if (tick_rate == medium_tick)
		tasks_medium.emplace_back(std::move(task));
	else if (tick_rate == fast_tick)
		tasks_fast.emplace_back(std::move(task));
	else
		throw std::invalid_argument{"Unsupported tick_rate"};
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
