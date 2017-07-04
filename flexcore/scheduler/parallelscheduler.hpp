#ifndef SRC_SCHEDULER_PARALLELSCHEDULER_HPP_
#define SRC_SCHEDULER_PARALLELSCHEDULER_HPP_

#include <flexcore/scheduler/scheduler.hpp>

#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>
#include <mutex>

namespace fc
{
namespace thread
{

/**
 * \brief simple scheduler based on a threadpool
 *
 * Adds tasks a task queue. These tasks are then assigned to worker threads in a pool
 *
 * \invariant thread_pool.size() > 0
 */
class parallel_scheduler : public scheduler
{
public:
	static int num_threads();

	parallel_scheduler();
	parallel_scheduler(const parallel_scheduler&) = delete;
	~parallel_scheduler() override;

	///adds a new task and notifies waiting threads.
	void add_task(task_t new_task) override;
	/// stops the work loop of all threads
	void stop() noexcept override;
	size_t nr_of_waiting_tasks() const override;

private:
	/// startes the work loop of all threads
	void start() noexcept;

	std::vector<std::thread> thread_pool;
	bool do_work; ///< flag indicates threads to keep working.

	// current implementation is simple and based on locking the task_queue,
	//might be worthwhile exchanging it for a lockfree one.
	std::queue<task_t> task_queue;
	mutable std::mutex task_queue_mutex;
	using queue_lock = std::unique_lock<std::mutex>;
	///used to notify worker threads if new tasks are available
	std::condition_variable thread_control;
};

} /* namespace thread */
} /* namespace fc */

#endif /* SRC_SCHEDULER_PARALLELSCHEDULER_HPP_ */
