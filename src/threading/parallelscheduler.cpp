#include "parallelscheduler.hpp"

#include <cassert>

namespace fc
{
namespace thread
{
const int parallel_scheduler::num_threads =  std::thread::hardware_concurrency();

parallel_scheduler::parallel_scheduler() :
		thread_pool(),
		do_work(true),
		task_queue()
{
	//fill thread_pool in body of constructor,
	//since otherwise threads would need to be copied
	for (int i=0; i != num_threads; ++i)
	{
		thread_pool.push_back(std::thread(
				//infinite job loop for every thread,
				//looks for jobs in task_queue and executes them
				[this] ()
				{
					while(do_work)
					{
						task_t job;
						{
							queue_lock lock(task_queue_mutex);
							if (!task_queue.empty())
							{
							job = task_queue.front();
							task_queue.pop();
							}
							else
								jobs_available.wait(lock); //todo currently sometimes deadlocks while waiting here
						} //releases lock
						if (job)
							job();
					}
				}));
	}
}

void parallel_scheduler::stop() noexcept
{
	do_work = false;
	queue_lock lock(task_queue_mutex);
	jobs_available.notify_all();
}

parallel_scheduler::~parallel_scheduler()
{
	//first stop the infinite loop in all threads
	stop();
	//then stop all calculations and join threads
	for (auto& thread : thread_pool)
		thread.join(); //todo currently sometimes deadlocks while trying to join thread waiting in line 36
}

size_t parallel_scheduler::nr_of_waiting_jobs()
{
	queue_lock lock(task_queue_mutex);
	return task_queue.size();
}

void parallel_scheduler::add_task(task_t new_task)
{
	queue_lock lock(task_queue_mutex);
	task_queue.push(new_task);
	jobs_available.notify_one();
}


} /* namespace thread */
} /* namespace fc */
