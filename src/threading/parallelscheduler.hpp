/*
 * parallelscheduler.h
 *
 *  Created on: Sep 23, 2015
 *      Author: ckielwein
 */

#ifndef SRC_THREADING_PARALLELSCHEDULER_HPP_
#define SRC_THREADING_PARALLELSCHEDULER_HPP_

#include <atomic>
#include <thread>
#include <vector>
#include <queue>
#include <cassert>
#include <mutex>

#include <iostream>

namespace fc
{
namespace thread
{

class parallel_scheduler
{
public:
	typedef std::function<void(void)> task_t;

	static const int num_threads;

	parallel_scheduler();
	parallel_scheduler(const parallel_scheduler&) = delete;
	~parallel_scheduler();

	void add_task(task_t new_task)
	{
		queue_lock lock;
		task_queue.push(new_task);
	}

	void start() noexcept { std::cout << "task_queue.size() start " << task_queue.size() << "\n";do_work = true; }
	void stop() noexcept { std::cout << "task_queue.size() stop " << task_queue.size() << "\n"; do_work = false;}
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
