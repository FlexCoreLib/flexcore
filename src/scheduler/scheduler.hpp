#ifndef SRC_THREADING_SCHEDULER_HPP_
#define SRC_THREADING_SCHEDULER_HPP_

#include <functional>

namespace fc
{
namespace thread
{
class scheduler
{
public:
	using task_t = std::function<void(void)>;
	virtual void add_task(task_t new_task) = 0;
	virtual void stop() = 0;
	virtual size_t nr_of_waiting_tasks() const = 0;
	virtual ~scheduler() = default;
};
} /* namespace thread */
} /* namespace fc */

#endif /* SRC_THREADING_SCHEDULER_HPP_ */
