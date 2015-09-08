// Scheduler.cpp ---
//
// Filename: Scheduler.cpp
// Description:
// Author: Thomas Karolski
// Created: Di Sep  8 21:10:40 2015 (+0200)
//
//
//

// Code:


#include "Scheduler.hpp"

bool Scheduler::ExecNextTask()
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


size_t Scheduler::ExecAllTasks()
{
	ClockTick();
	size_t count(0);
	while(ExecNextTask()) { ++count; }
	return count;
}



//
// Scheduler.cpp ends here
