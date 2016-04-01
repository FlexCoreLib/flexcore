#include "serialschedulers.hpp"

namespace fc
{
namespace thread
{

void blocking_scheduler::add_task(task_t new_task)
{
	std::lock_guard<std::mutex> lock(mutex);
	if (stopped)
		throw std::runtime_error{"attempting to add a task to stopped scheduler."};
	new_task();
}

void blocking_scheduler::stop()
{
	std::lock_guard<std::mutex> lock(mutex);
	stopped = true;
}

size_t blocking_scheduler::nr_of_waiting_tasks() const
{
	if (!mutex.try_lock())
		return 1;
	mutex.unlock();
	return 0;
}

blocking_scheduler::~blocking_scheduler()
{
	stop();
}

} // namespace thread
} // namespace fc

