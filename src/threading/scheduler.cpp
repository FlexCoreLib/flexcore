#include "scheduler.hpp"

namespace fc
{

bool scheduler::ExecNextTask()
{
	bool result(false);
	PreTaskHook();

	if(!Empty())
	{
		boost::type_erasure::any<CallableRequirements> fn = mTaskQueue.front();
		mTaskQueue.pop();
		fn();
		result = true;
	}
	else
		result = false;

	PostTaskHook();

	return result;
}

size_t scheduler::ExecAllTasks()
{
	ClockTick();
	size_t count(0);
	while(ExecNextTask()) { ++count; }
	return count;
}

} // namespace fc
