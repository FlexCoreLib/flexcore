#ifndef SRC_THREADING_CYCLECONTROL_HPP_
#define SRC_THREADING_CYCLECONTROL_HPP_

#include <clock/clock.hpp>
#include <ports/events/event_sources.hpp>
#include "scheduler.hpp"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>

namespace fc
{
namespace thread
{

struct condition_pair
{
	std::mutex mtx;
	std::condition_variable cv;
};

/**
 * \brief class representing a periodic task.
 */
struct periodic_task final
{
	/**
	 * \brief Constructor taking a job and the cycle rate.
	 * \param job task which is to be executed every cycle
	 */
	periodic_task(std::function<void(void)> job) :
				work_to_do(false),
				sync(std::make_unique<condition_pair>()),
				work(job)
	{
	}

	bool done()
	{
		std::lock_guard<std::mutex> lock(sync->mtx);
		return !work_to_do;
	}

	void set_work_to_do(bool todo)
	{
		{
			std::lock_guard<std::mutex> lock(sync->mtx);
			work_to_do = todo;
		}
		// if we're done then notify all waiters
		if (todo == false)
			sync->cv.notify_all();
	}

	/** \brief waits for this task to be done, but only until the provided timeout.
	 * \return true if the task is done.
	 */
	bool wait_until_done(virtual_clock::steady::duration timeout)
	{
		std::unique_lock<std::mutex> lock(sync->mtx);
		return sync->cv.wait_for(lock, timeout, [&]
		                         {
			                         return !work_to_do;
		                         });
	}

	void send_switch_tick() { switch_tick.fire(); }
	auto& out_switch_tick() { return switch_tick; }

	void operator()()
	{
		work();
		set_work_to_do(false);
	}
private:
	/// flag to check if work has already been executed this cycle.
	bool work_to_do;
	std::unique_ptr<condition_pair> sync;
	/// work to be done every cycle
	std::function<void(void)> work;

	//Todo refactor this intrusion of ports into otherwise independent code
	pure::event_source<void> switch_tick;
};

/**
 * \brief Controls timing and the execution of cyclic tasks in the scheduler.
 * Todo: allow to set virtual clock as control clock for replay as template parameter
 * todo: allow to set min_tick_length
 */
class cycle_control
{
public:
	static constexpr wall_clock::steady::duration min_tick_length =
			wall_clock::steady::duration(std::chrono::milliseconds(10));

	static constexpr virtual_clock::steady::duration fast_tick = min_tick_length;
	static constexpr virtual_clock::steady::duration medium_tick = min_tick_length * 10;
	static constexpr virtual_clock::steady::duration slow_tick = min_tick_length * 100;

	explicit cycle_control(std::unique_ptr<scheduler> scheduler);
	~cycle_control();

	/// starts the main loop
	void start(bool fast=false);
	/// stops the main loop in all threads
	void stop();

	/// advances the clock by a single tick and executes all tasks for the cycle.
	void work();

	/**
	 * \brief adds a new cyclic task with the given tick_rate.
	 * Tasks can only be added as long as the cycle_control has not been started. A
	 * std::runtime_error exception will be thrown if an attempt is made to add a task to a running
	 * cycle_control.
	 *
	 * \pre cycle_control is not running
	 * \post list of tasks for given tick_rate is not empty
	 */
	void add_task(periodic_task task, virtual_clock::duration tick_rate);
	size_t nr_of_tasks() { return scheduler_->nr_of_waiting_tasks(); }

	std::exception_ptr last_exception();

private:
	/// normal main loop, which run tasks and sticks to tick lengths
	void normal_main_loop();
	/// accelerated main loop, which maintains ratios of execution counts of tasks
	void fast_main_loop();
	/// runs the tasks in this vector; returns false if any task is not done, true otherwise
	bool run_periodic_tasks(std::vector<periodic_task>& tasks);
	void wait_for_current_tasks();
	std::vector<periodic_task> tasks_slow;
	std::vector<periodic_task> tasks_medium;
	std::vector<periodic_task> tasks_fast;
	std::unique_ptr<scheduler> scheduler_;
	std::atomic<bool> keep_working{false};
	bool running = false;
	//Todo refactor main loop and task queue to locked class together
	std::thread main_loop_thread;

	//Thread exception handling
	std::mutex task_exception_mutex;
	std::deque<std::exception_ptr> task_exceptions;
	void store_exception();
};

} /* namespace thread */

struct out_of_time_exception: std::runtime_error
{
	out_of_time_exception() :
			std::runtime_error("cyclic task has not finished in time")
	{
	}
};
} /* namespace fc */

#endif /* SRC_THREADING_CYCLECONTROL_HPP_ */
