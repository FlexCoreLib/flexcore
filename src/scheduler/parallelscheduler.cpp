#include <scheduler/parallelscheduler.hpp>

#include <cassert>
#include <utility>

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
		do_work(false),
		task_queue()
{
	start();
}


void parallel_scheduler::start() noexcept
{
	do_work = true;

	//fill thread_pool in body of constructor,
	//since otherwise threads would need to be copied
	for (int i = 0, e = num_threads(); i != e; ++i)
	{
		thread_pool.push_back(std::thread(
				//infinite task loop for every thread,
				//looks for tasks in task_queue and executes them
				[this] ()
				{
					while (true)
					{
						task_t task;
						{
							queue_lock lock(task_queue_mutex);
							// Wait while task_queue is empty and do_work is true.
							// If do_work is false then exit loop. If task_queue is not empty, exit.
							// Still need to check which condition is true after the while loop.
							while (task_queue.empty() && do_work)
								thread_control.wait(lock);

							if (!do_work)
								return;

							// if we're here, then do_work must be true, and
							// task_queue.empty() must be false.
							assert(do_work);
							assert(!task_queue.empty());

							task = task_queue.front();
							task_queue.pop();
						}
						if (task)
							task();
					}
				}));
	}
	assert(!thread_pool.empty()); //check invariant
}

void parallel_scheduler::stop() noexcept
{
	//first stop the infinite loop in all threads
	{
		//Acquire lock first, to stop work loops to enter while we set the flag.
		queue_lock lock(task_queue_mutex);
		do_work = false;
	}
	thread_control.notify_all();
	//then stop all calculations and join threads
	for (auto& thread : thread_pool)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
	assert(!thread_pool.empty()); //check invariant
}

parallel_scheduler::~parallel_scheduler()
{
	//first stop all threads, destroying running threads is illegal
	stop();
}

size_t parallel_scheduler::nr_of_waiting_tasks() const
{
	queue_lock lock(task_queue_mutex);
	return task_queue.size();
}

void parallel_scheduler::add_task(task_t new_task)
{
	{
		queue_lock lock(task_queue_mutex);
		task_queue.push(std::move(new_task));
	}
	thread_control.notify_one();
	assert(!thread_pool.empty()); //check invariant
}


} /* namespace thread */
} /* namespace fc */
