#ifndef SRC_THREADING_CYCLECONTROL_HPP_
#define SRC_THREADING_CYCLECONTROL_HPP_

#include <vector>

#include <clock/clock.hpp>
#include <ports/event_ports.hpp>

#include "parallelscheduler.hpp"

namespace fc
{
namespace thread
{

/**
 * \brief class representing a periodic task with cycle rate
 */
struct periodic_task final
{
	/**
	 * \brief Constructor taking a job and the cycle rate.
	 * \param job task which is to be executed every cycle
	 * \param cycle_duration duration of one cycle of the task in virtual time
	 */
	periodic_task(std::function<void(void)> job,
			virtual_clock::duration cycle_duration) :
				work_to_do(std::make_shared<bool>(false)),
				interval(cycle_duration),
				work(job)
	{
	}

	bool done() const { return !(*work_to_do); }
	void set_work_to_do(bool todo) { *work_to_do = todo; }
	bool is_due(virtual_clock::steady::time_point now) const;
	void send_switch_tick() { switch_tick.fire(); }
	auto out_switch_tick() { return switch_tick; }

	void operator()()
	{
		work();
		set_work_to_do(false);
	}
private:
	/// flag to check if work has already been executed this cycle.
	std::shared_ptr<bool> work_to_do;
	virtual_clock::duration interval;
	/// work to be done every cycle
	std::function<void(void)> work;

	//Todo refactor this intrusion of ports into otherwise independent code
	event_out_port<void> switch_tick;
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
	size_t nr_of_tasks() { return scheduler.nr_of_waiting_tasks(); }

private:
	/// contains the main loop, which is running as as long as it is not stopped
	void main_loop();
	void run_periodic_tasks();
	std::vector<periodic_task> tasks;
	parallel_scheduler scheduler;
	bool keep_working = false;
	std::condition_variable main_loop_control;
	//Todo refactor main loop and task queue to locked class together with their mutex
	std::mutex main_loop_mutex;
	std::mutex task_queue_mutex;
	std::thread main_loop_thread;
};

} /* namespace thread */
} /* namespace fc */

#endif /* SRC_THREADING_CYCLECONTROL_HPP_ */
