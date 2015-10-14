#include "parallelscheduler.hpp"

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
								jobs_available.wait(lock);
						} //releases lock
						if (job)
							job();
					}
				}));
	}
}

parallel_scheduler::~parallel_scheduler()
{
	//first stop the infinite loop in all threads
	do_work = false;
	jobs_available.notify_all();
	//then stop all calculations and join threads
	for (auto& thread : thread_pool)
		thread.join();
}

size_t parallel_scheduler::nr_of_waiting_jobs()
{
	queue_lock lock(task_queue_mutex);
	return task_queue.size();
}

} /* namespace thread */
} /* namespace fc */
