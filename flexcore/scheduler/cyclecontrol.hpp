#ifndef SRC_THREADING_CYCLECONTROL_HPP_
#define SRC_THREADING_CYCLECONTROL_HPP_

#include <flexcore/scheduler/clock.hpp>
#include <flexcore/scheduler/scheduler.hpp>
#include <flexcore/scheduler/parallelregion.hpp>
#include <flexcore/pure/event_sources.hpp>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>

namespace fc
{

/// Classes and Functions related to the multithreading model of flexcore.
namespace thread
{
namespace detail
{
struct condition_pair
{
	std::mutex mtx;
	std::condition_variable cv;
};
}

/**
 * \brief class representing a task
 * which is executed with a fixed rate by the scheduler.
 */
struct periodic_task final
{
	/**
	 * \brief Constructor taking a job
	 * \param job task which is to be executed every cycle
	 * \pre job must not be empty
	 */
	explicit periodic_task(std::function<void(void)> job)
		: work_to_do(false)
		, sync(std::make_unique<detail::condition_pair>())
		, work(std::move(job))
		, work_start(wall_clock::steady::now())
		, region(nullptr)
	{
		assert(work);
	}
	/// Construct a periodic task executes work within a region
	explicit periodic_task(const std::shared_ptr<parallel_region>& r) :
				work_to_do(false),
				sync(std::make_unique<detail::condition_pair>()),
				work(r->ticks.in_work()),
				work_start(wall_clock::steady::now()),
				region(r)
	{
		assert(r != nullptr);
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
		return sync->cv.wait_until(
				lock,
				work_start + timeout,
				[this](){ return !this->work_to_do; }
		);
	}

	void send_switch_tick()
	{
		if (region)
			region->ticks.switch_buffers();
	}

	void operator()()
	{
		work_start = wall_clock::steady::now();
		work();
		set_work_to_do(false);
	}
private:
	/// flag to check if work has already been executed this cycle.
	bool work_to_do;
	std::unique_ptr<detail::condition_pair> sync;
	/// work to be done every cycle
	std::function<void(void)> work;
	/// start time of most recent work cycle
	wall_clock::steady::time_point work_start;

	std::shared_ptr<parallel_region> region;
};

///Abstract Base class for all main lopp classes.
class main_loop
{
public:
	virtual void loop_body(const std::function<void(void)>& work) = 0;
	virtual ~main_loop() = default;

	virtual void arm() = 0;

	std::function<void(void)> wait_for_current_tasks{};
};

/**
 * \brief Main Loop which runs as fast as possible
 */
class afap_main_loop : public main_loop
{
public:
	void loop_body(const std::function<void(void)>& work) override;

	void arm() override {};

};

/**
 * \brief Main Loop which runs in realtime.
 */
class realtime_main_loop : public main_loop
{
public:
	void loop_body(const std::function<void(void)>& work) override;

	void arm() override { epoch = wall_clock::steady::now(); };

private:
	wall_clock::steady::time_point epoch{wall_clock::steady::now()};
};

/**
 * \brief Main Loop which runs variable speed.
 */
class timewarp_main_loop : public main_loop
{
public:
	void loop_body(const std::function<void(void)>& work) override;

	void set_warp_factor(double factor);

	void arm() override { epoch = wall_clock::steady::now(); };
private:
	std::mutex warp_mutex{};
	std::condition_variable warp_signal{};
	double warp_factor{1};
	wall_clock::steady::time_point epoch{wall_clock::steady::now()};
};

/**
 * \brief Controls timing and the execution of cyclic tasks in the scheduler.
 * Todo: allow to set virtual clock as control clock for replay as template parameter
 * todo: allow to set min_tick_length
 */
class cycle_control
{
public:
	//forward definitions of tick rates for use in scheduler
	static constexpr auto min_tick_length = parallel_region::min_tick_length;
	static constexpr auto fast_tick =  parallel_region::fast_tick;
	static constexpr auto medium_tick =  parallel_region::medium_tick;
	static constexpr auto slow_tick =  parallel_region::slow_tick;

	/**
	 * \brief construct cycle_control with a scheduler and a main loop functor
	 * \param scheduler scheduler for work ticks to be used.
	 * \param loop functor which controls the main loop
	 * \pre loop != nullptr
	 *
	 * \see parallel_scheduler for an example of a scheduler.
	 */
	explicit cycle_control(std::unique_ptr<scheduler> scheduler,
			const std::shared_ptr<main_loop>& loop = std::make_shared<realtime_main_loop>());

	/**
	 * \brief Construct cycle_control together with a user controled timeout handler
	 * \param scheduler scheduler for work ticks to be used.
	 * \param loop functor which controls the main loop
	 * \param err TimeOutHandler, is triggered when the scheduler notices
	 * that a work tick takes to long.
	 * \pre loop != nullptr
	 */
	template <class TimeOutFun>
	cycle_control(std::unique_ptr<scheduler> scheduler,
			TimeOutFun err,
			const std::shared_ptr<main_loop>& loop);

	~cycle_control();

	/// starts the main loop
	void start();
	/// halts the main loop without joining worker threads
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
	/// returns the number of currently scheduled tasks
	size_t nr_of_tasks() const { return scheduler_->nr_of_waiting_tasks(); }

	/// Get last exception thrown by timeout. Returns nullptr if no exception was thrown
	std::exception_ptr last_exception();

	void set_main_loop(const std::shared_ptr<main_loop>& loop);

private:
	struct tick_task_pair
	{
		virtual_clock::steady::duration tick;
		std::vector<periodic_task> tasks{};
		std::vector<std::reference_wrapper<periodic_task>> done_tasks{};
	};

	/// runs the tasks in this vector; returns false if any task is not done, true otherwise
	bool run_periodic_tasks(tick_task_pair& tasks);
	void wait_for_current_tasks();

	tick_task_pair tasks_slow{slow_tick};
	tick_task_pair tasks_medium{medium_tick};
	tick_task_pair tasks_fast{fast_tick};
	std::unique_ptr<scheduler> scheduler_;
	std::atomic<bool> keep_working{false};
	bool running = false;

	std::shared_ptr<main_loop> main_loop_;
	std::thread main_loop_thread;

	//Thread exception handling
	std::mutex task_exception_mutex;
	std::deque<std::exception_ptr> task_exceptions;
	/** Callback that is called when a task takes too long.
	 * Expected to return true if the scheduler is to continue and false if
	 * scheduler should shut itself down.
	 */
	std::function<bool(periodic_task&)> timeout_callback;
	bool store_exception(periodic_task& task);
};

template <class TimeOutFun>
inline cycle_control::cycle_control(std::unique_ptr<scheduler> scheduler,
		TimeOutFun callback, const std::shared_ptr<main_loop>& loop)
	: scheduler_(std::move(scheduler))
	, main_loop_(loop)
	, timeout_callback(std::move(callback))
{
	assert(scheduler_);
	main_loop_->wait_for_current_tasks = [this](){ wait_for_current_tasks(); };
}

} /* namespace thread */

struct out_of_time_exception: std::runtime_error
{
	out_of_time_exception() :
			std::runtime_error("cyclic task has not finished in time")
	{
	}
};

} /* namespace fc */

#endif /* SRC_SCHEDULER_CYCLECONTROL_HPP_ */
