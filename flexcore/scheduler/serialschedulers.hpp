#ifndef SRC_THREADING_SERIALSCHEDULER_HPP_
#define SRC_THREADING_SERIALSCHEDULER_HPP_

#include <flexcore/scheduler/scheduler.hpp>
#include <mutex>

namespace fc
{
namespace thread
{
/// A scheduler that blocks and executes a task as soon as it is added.
class blocking_scheduler : public scheduler
{
public:
	void add_task(task_t new_task) override;
	void stop() override;
	size_t nr_of_waiting_tasks() const override;
	~blocking_scheduler() override;

	blocking_scheduler() = default;
private:
	mutable std::mutex mutex;
	bool stopped = false;
};
} /* namespace thread */
} /* namespace fc */

#endif /* SRC_THREADING_SERIALSCHEDULER_HPP_ */
