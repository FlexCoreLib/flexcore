#include "cyclecontrol.hpp"

#include <stdexcept>
#include <cassert>
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
	// the mutex needs to be held to check the condition; waiting releases the lock and reaquires it
	// afterwards.
	std::unique_lock<std::mutex> loop_lock(main_loop_mutex);
	while (keep_working)
	{
		auto now = wall_clock::steady::now();
		const int slow_medium_ratio = slow_tick / medium_tick;
		const int medium_fast_ratio = medium_tick / fast_tick;

		if (!run_periodic_tasks(tasks_slow)) return;

		for (int slow_iter = 0; slow_iter < slow_medium_ratio; ++slow_iter)
		{
			if (!run_periodic_tasks(tasks_medium)) return;

			for (int medium_iter = 0; medium_iter < medium_fast_ratio; ++medium_iter)
			{
				if (!run_periodic_tasks(tasks_fast)) return;

				// !keep_working is the predicate.
				// When the predicate returns true (*don't keep working*) the
				// wait_for returns true and we return.
				// When the timeout comes the value of the predicate is
				// returned, which *should* be false. If the predicate is true at
				// the timeout then it means keep_working is false and we should
				// return.
				//
				// Thus we either return or keep_working is true.
				if (main_loop_control.wait_until(loop_lock, now + min_tick_length, [this]
				                                 {
					                                 return !keep_working;
				                                 }))
					return;
				assert(keep_working);
				clock::advance();
				now = wall_clock::steady::now();
			}
		}
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

	std::vector<periodic_task>* v = nullptr;
	if (tick_rate == slow_tick)
		v = &tasks_slow;
	else if (tick_rate == medium_tick)
		v = &tasks_medium;
	else if (tick_rate == fast_tick)
		v = &tasks_fast;
	else
		throw std::invalid_argument{"Unsupported tick_rate"};

	assert(v);
	v->emplace_back(std::move(task));
	assert(!v->empty());
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
