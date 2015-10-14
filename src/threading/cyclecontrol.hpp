#ifndef SRC_THREADING_CYCLECONTROL_HPP_
#define SRC_THREADING_CYCLECONTROL_HPP_

#include "parallelscheduler.hpp"
#include <clock/clock.hpp>

#include <vector>

namespace fc
{
namespace thread
{

/**
 * \brief class representing a periodic task with cycle rate
 */
struct periodic_task
{
	/**
	 * \brief Constructor taking a job and the cycle rate.
	 * \param job task which is to be executed every cycle
	 * \param rate cycle rate of the task in virtual time
	 */
	periodic_task(std::function<void(void)> job,
			virtual_clock::duration rate) :
				work_to_do(false),
				cycle_rate(rate),
				work(job)
	{
	}

	bool done() const { return !work_to_do; }
	void set_work_to_do(bool todo) { work_to_do = todo; }
	bool is_due(virtual_clock::duration time) const;
	void operator()()
	{
		work_to_do = false;
		work();
	}
private:
	/// flag to check if work has already been executed this cycle.
	bool work_to_do;
	virtual_clock::duration cycle_rate;
	/// work to be done every cycle
	std::function<void(void)> work;
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

	cycle_control() = default;
	~cycle_control();

	/// starts the main loop
	void start();
	/// stops the main loop in all threads
	void stop();

	/// advances the clock by a single tick and executes all tasks for a single cycle.
	void work();

	/**
	 * \brief adds a new cyclic task.
	 * \post list of tasks is not empty
	 */
	void add_task(periodic_task task);
	size_t nr_of_tasks() { return scheduler.nr_of_waiting_jobs(); }
private:
	/// contains the main loop, which is running as as long as it is not stopped
	void main_loop();
	void run_periodic_tasks();
	std::vector<periodic_task> tasks;
	parallel_scheduler scheduler;
	bool keep_working = false;
	std::condition_variable main_loop_control;
	std::mutex main_loop_mutex;
	std::mutex task_queue_mutex;
	std::thread main_loop_thread;
};

} /* namespace thread */
} /* namespace fc */

#endif /* SRC_THREADING_CYCLECONTROL_HPP_ */
