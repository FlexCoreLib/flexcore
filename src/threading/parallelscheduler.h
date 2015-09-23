/*
 * parallelscheduler.h
 *
 *  Created on: Sep 23, 2015
 *      Author: ckielwein
 */

#ifndef SRC_THREADING_PARALLELSCHEDULER_H_
#define SRC_THREADING_PARALLELSCHEDULER_H_

#include <thread>
#include <vector>
#include <queue>
#include <cassert>

namespace fc
{
namespace thread
{

class parallel_scheduler
{
public:
	typedef std::function<void(void)> task_t;

	void add_task(task_t new_task)
	{
		task_queue.push(new_task);
	}

	bool empty() const noexcept { return task_queue.empty(); }

	void execute_tasks()
	{
		distribute_tasks();
		assert(task_queue.empty());
	}

private:
	void distribute_tasks(); //ToDo

	std::vector<std::thread> thread_pool;
	std::queue<task_t> task_queue;
};

} /* namespace thread */
} /* namespace fc */

#endif /* SRC_THREADING_PARALLELSCHEDULER_H_ */
