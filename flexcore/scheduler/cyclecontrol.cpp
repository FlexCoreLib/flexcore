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

cycle_control::cycle_control(std::unique_ptr<scheduler> scheduler,
		 const std::shared_ptr<main_loop>& loop)
	: cycle_control(std::move(scheduler), [this](auto& task)
					{
						return store_exception(task);
					},
					loop
	)
{
}

void cycle_control::start()
{
	assert(!running);
	keep_working.store(true);
	running = true;
	// give the main thread some actual work to do (execute infinite main loop)
	auto worker = [this](){ work(); };
	main_loop_thread = std::thread{
		[&, worker](){
			while(keep_working.load())
				main_loop_->loop_body([worker](){worker();});
		}
	};
}

void cycle_control::stop()
{
	keep_working.store(false);
	if (main_loop_thread.joinable())
		main_loop_thread.join();
	// wait for scheduled tasks to finish
	auto wait_or_throw = [](auto& task_vector)
	{
		for (auto& t : task_vector.tasks)
			if (!t.wait_until_done(slow_tick))
				throw out_of_time_exception{};
	};
	wait_or_throw(tasks_fast);
	wait_or_throw(tasks_medium);
	wait_or_throw(tasks_slow);
	running = false;
}

bool cycle_control::store_exception(periodic_task&)
{
	auto ep = std::make_exception_ptr(out_of_time_exception());
	std::lock_guard<std::mutex> lock(task_exception_mutex);
	task_exceptions.push_back(ep);
	return false;
}

void cycle_control::work()
{
	auto now = virtual_clock::steady::now().time_since_epoch();
	auto run_if_due = [this, now](auto& task_vector)
	{
		if (now % task_vector.tick == virtual_clock::duration::zero())
			if (!run_periodic_tasks(task_vector.tasks))
				return false;
		return true;
	};
	clock::advance();
	if (!run_if_due(tasks_fast)) return;
	if (!run_if_due(tasks_medium)) return;
	if (!run_if_due(tasks_slow)) return;
}

void cycle_control::wait_for_current_tasks()
{
	auto now = virtual_clock::steady::now().time_since_epoch();
	auto wait_for_tasks = [this, now](auto& task_vector)
	{
		if (now % task_vector.tick == virtual_clock::duration::zero())
		{
			for (auto& task : task_vector.tasks)
				if (!task.wait_until_done(task_vector.tick))
				{
					if (!timeout_callback(task))
					{
						keep_working.store(false);
						return false;
					}
				}
		}
		return true;
	};
	if (!wait_for_tasks(tasks_slow)) return;
	if (!wait_for_tasks(tasks_medium)) return;
	if (!wait_for_tasks(tasks_fast)) return;
}

cycle_control::~cycle_control()
{
	stop();
}

bool cycle_control::run_periodic_tasks(std::vector<periodic_task>& tasks)
{
	std::vector<std::reference_wrapper<periodic_task>> done_tasks;
	done_tasks.reserve(tasks.size());
	for (auto& task : tasks)
		if (!task.done())
		{
			if (!timeout_callback(task))
			{
				keep_working.store(false);
				return false;
			}
		}
		else
		{
			done_tasks.emplace_back(task);
		}

	for (auto& task_ref : done_tasks)
	{
		periodic_task& task = task_ref.get();
		assert(task.done());
		task.set_work_to_do(true);
		task.send_switch_tick();
	}
	for (auto& task_ref : done_tasks)
	{
		periodic_task& task = task_ref.get();
		scheduler_->add_task([&task] { task(); });
	}
	return true;
}

void cycle_control::add_task(periodic_task task, virtual_clock::duration tick_rate)
{
	if (running)
		throw std::runtime_error{"Worker threads are already running"};

	if (tick_rate == slow_tick)
		tasks_slow.tasks.emplace_back(std::move(task));
	else if (tick_rate == medium_tick)
		tasks_medium.tasks.emplace_back(std::move(task));
	else if (tick_rate == fast_tick)
		tasks_fast.tasks.emplace_back(std::move(task));
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

void cycle_control::set_main_loop(const std::shared_ptr<main_loop>& loop)
{
	assert(!running);
	main_loop_ = loop;
	main_loop_->wait_for_current_tasks = [this](){ wait_for_current_tasks(); };
}

void realtime_main_loop::loop_body(std::function<void(void)> work)
{
	const auto now = wall_clock::steady::now();
	work();
	std::this_thread::sleep_until(now + cycle_control::min_tick_length);
}

void timewarp_main_loop::loop_body(std::function<void(void)> work)
{
	const auto now = wall_clock::steady::now();
	wait_for_current_tasks();
	work();

	std::unique_lock<std::mutex> lock(warp_mutex);
	warp_signal.wait_until(lock,
			now + cycle_control::min_tick_length * warp_factor,
			[this, &now]()
			{
				return wall_clock::steady::now() >= now + cycle_control::min_tick_length * warp_factor;
			});
}

void timewarp_main_loop::set_warp_factor(double factor)
{
	std::lock_guard<std::mutex> lock(warp_mutex);
	warp_factor = factor;
	warp_signal.notify_all();
}

void afap_main_loop::loop_body(std::function<void(void)> work)
{
	wait_for_current_tasks();
	work();
}

} /* namespace thread */
} /* namespace fc */
