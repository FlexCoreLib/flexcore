#include "parallelscheduler.hpp"

#include <cassert>

namespace fc
{
namespace thread
{
int parallel_scheduler::num_threads()
{
		return std::max(1u ,std::thread::hardware_concurrency());
}

parallel_scheduler::parallel_scheduler() :
		thread_pool(),
		do_work(true),
		task_queue()
{
	//fill thread_pool in body of constructor,
	//since otherwise threads would need to be copied
	for (int i=0; i != num_threads(); ++i)
	{
		thread_pool.push_back(std::thread(
				//infinite task loop for every thread,
				//looks for tasks in task_queue and executes them
				[this] ()
				{
					while(do_work)
					{
						task_t task;
						{
							queue_lock lock(task_queue_mutex);
							if (!task_queue.empty())
							{
								task = task_queue.front();
								task_queue.pop();
							} // else do nothing
						} //releases lock
						if (task)
							task();
					}
				}));
	}
}

void parallel_scheduler::stop() noexcept
{
	//first stop the infinite loop in all threads
	do_work = false;
	//then stop all calculations and join threads
	for (auto& thread : thread_pool)
	{
		if (thread.joinable())
			thread.join();
	}
}

parallel_scheduler::~parallel_scheduler()
{
	stop();
}

size_t parallel_scheduler::nr_of_waiting_tasks()
{
	queue_lock lock(task_queue_mutex);
	return task_queue.size();
}

void parallel_scheduler::add_task(task_t new_task)
{
	queue_lock lock(task_queue_mutex);
	task_queue.push(new_task);
}


} /* namespace thread */
} /* namespace fc */
