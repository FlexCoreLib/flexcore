#ifndef SRC_THREADING_PARALLELSCHEDULER_HPP_
#define SRC_THREADING_PARALLELSCHEDULER_HPP_

#include <atomic>
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
 */
class parallel_scheduler
{
public:
	typedef std::function<void(void)> task_t;

	static const int num_threads;

	parallel_scheduler();
	parallel_scheduler(const parallel_scheduler&) = delete;
	~parallel_scheduler();

	///adds a new task and notifies waiting threads.
	void add_task(task_t new_task);

	/// startes the work loop of all threads
	void start() noexcept { do_work = true; }
	/// stops the work loop of all threads
	void stop() noexcept;

	size_t nr_of_waiting_jobs();

private:
	std::vector<std::thread> thread_pool;
	std::atomic<bool> do_work; // flag indicates threads to keep working.

	// current implementation is simple and based on locking the task_queue,
	//might be worthwhile exchanging it for a lockfree one.
	std::queue<task_t> task_queue;
	std::mutex task_queue_mutex;
	typedef std::unique_lock<std::mutex> queue_lock;
};

} /* namespace thread */
} /* namespace fc */

#endif /* SRC_THREADING_PARALLELSCHEDULER_HPP_ */
