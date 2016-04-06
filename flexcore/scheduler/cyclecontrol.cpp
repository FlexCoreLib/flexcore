#include <flexcore/scheduler/cyclecontrol.hpp>

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

cycle_control::cycle_control(std::unique_ptr<scheduler> scheduler) : scheduler_(std::move(scheduler))
{
	assert(scheduler_);
}

void cycle_control::start(bool fast)
{
	keep_working.store(true);
	running = true;
	// give the main thread some actual work to do (execute infinite main loop)
	if (fast)
		main_loop_thread = std::thread{[this] { fast_main_loop(); }};
	else
		main_loop_thread = std::thread{[this] { normal_main_loop(); }};
}

void cycle_control::stop()
{
	keep_working.store(false);
	if (main_loop_thread.joinable())
		main_loop_thread.join();
	scheduler_->stop();
	running = false;
}

void cycle_control::store_exception()
{
	auto ep = std::make_exception_ptr(out_of_time_exception());
	std::lock_guard<std::mutex> lock(task_exception_mutex);
	task_exceptions.push_back(ep);
	keep_working.store(false);
}

void cycle_control::work()
{
	auto now = virtual_clock::steady::now().time_since_epoch();
	auto run_if_due = [this, now](auto tick_rate, auto& task_vector)
	{
		if (now % tick_rate == virtual_clock::duration::zero())
			if (!run_periodic_tasks(task_vector))
				return false;
		return true;
	};
	if (!run_if_due(slow_tick, tasks_slow)) return;
	if (!run_if_due(medium_tick, tasks_medium)) return;
	if (!run_if_due(fast_tick, tasks_fast)) return;
	clock::advance();
}

void cycle_control::wait_for_current_tasks()
{
	auto now = virtual_clock::steady::now().time_since_epoch();
	auto wait_for_tasks = [this, now](auto tick_rate, auto& task_vector)
	{
		if (now % tick_rate == virtual_clock::duration::zero())
		{
			for (auto& task : task_vector)
				if (!task.wait_until_done(tick_rate))
				{
					store_exception();
					return false;
				}
		}
		return true;
	};
	if (!wait_for_tasks(slow_tick, tasks_slow)) return;
	if (!wait_for_tasks(medium_tick, tasks_medium)) return;
	if (!wait_for_tasks(fast_tick, tasks_fast)) return;
}

void cycle_control::fast_main_loop()
{
	while (keep_working.load())
	{
		wait_for_current_tasks();
		work();
	}
}

void cycle_control::normal_main_loop()
{
	while (keep_working.load())
	{
		const auto now = wall_clock::steady::now();
		work();
		std::this_thread::sleep_until(now + min_tick_length);
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
		store_exception();
		return false;
	}

	for (auto& task : tasks)
	{
		task.set_work_to_do(true);
		task.send_switch_tick();
		scheduler_->add_task([&task] { task(); });
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
