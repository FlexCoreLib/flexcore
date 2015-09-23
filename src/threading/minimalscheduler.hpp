#ifndef SRC_THREADING_MINIMALSCHEDULER_H_
#define SRC_THREADING_MINIMALSCHEDULER_H_

#include <queue>
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/callable.hpp>

#include <core/traits.hpp>

namespace fc
{

template<class Callable, class Enable = void >
struct is_callable_concept
{
	static void apply(Callable& callable) { callable(); }
};


/// Basic, single threaded scheduler. Takes tasks and provides a means to execute them.
/** Takes anything that is copy_constructible and callable as a task. */
class minimal_scheduler
{
public:
	typedef boost::mpl::vector< boost::type_erasure::copy_constructible<>,
								boost::type_erasure::callable<void()>
								> CallableRequirements;

	minimal_scheduler() {}
	virtual ~minimal_scheduler() {}

	/// Add a task to be executed in fifo order.
	/** The task must adhere to the boost::Generator concept. */
	template<class Callable>
	void AddTask(Callable task)
	{
		BOOST_CONCEPT_ASSERT((boost::Generator<Callable, void>));
		BOOST_CONCEPT_ASSERT((boost::CopyConstructible<Callable>));
		AddTask(boost::type_erasure::any<CallableRequirements>(task));
	}

	/// Add a task to be executed in fifo order.
	/** Like AddTask(Callable task), but uses type erasure with
	 * requirements from CallableRequirements. */
	void AddTask(boost::type_erasure::any<CallableRequirements> callable) {mTaskQueue.push(callable);}

	/// Returns true if there are no tasks queued
	bool Empty() const { return mTaskQueue.empty(); }

	/// Returns count of currently queued tasks
	size_t TaskCount() const { return mTaskQueue.size(); }

	/// Run prior to running a task (use for measurements?)
	virtual void PreTaskHook() {}
	/// Run after running a task (use for measurements?)
	virtual void PostTaskHook() {}

	/// Forward virtual clock by one tick
	virtual void ClockTick() {}

	/// Execute the next task in the queue
	bool ExecNextTask();

	/// Execute all tasks and return the amount of tasks executed
	size_t ExecAllTasks();

private:
	/// Queue of type-erased callables. We can put anything in here that adheres to the requirements
	std::queue< boost::type_erasure::any<CallableRequirements > > mTaskQueue;
};

} // namespace fc

#endif // SRC_THREADING_MINIMALSCHEDULER_H_


