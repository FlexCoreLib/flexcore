// TestScheduler.cpp ---
//
// Filename: TestScheduler.cpp
// Description:
// Author: Thomas Karolski
// Created: Di Sep  8 18:59:00 2015 (+0200)
//
//
//

// Code:


// boost
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>

#include "../../src/threading/scheduler.hpp"

using namespace fc;

class IncOp
{
public:
	IncOp(size_t& val, size_t delta = 1) : mVal(val), mDelta(delta) {}
	virtual ~IncOp() {}

	void operator()() { Inc(); }
	void Inc() {mVal += mDelta;}

private:
	size_t& mVal;
	size_t mDelta;
};

static void IncOpFn(size_t& val, int delta = 3) { val += delta; }


BOOST_AUTO_TEST_CASE( test_scheduler_possible_callables)
{ // test what can be inserted as a task
	scheduler scheduler;
	size_t state(0);
	IncOp incrementer(state, 2);
	scheduler.AddTask(IncOp(state));								// class with operator()()
	scheduler.AddTask(boost::bind(&IncOp::Inc, &incrementer));		// boost::bind to member fn
	scheduler.AddTask([&state]() { state += 3; });					// anonym fn
	scheduler.AddTask(boost::bind(IncOpFn, boost::ref(state), 4));	// boost bind static fn

	scheduler.ExecNextTask();
	BOOST_CHECK(state == 1);

	scheduler.ExecNextTask();
	BOOST_CHECK(state == 3);

	scheduler.ExecNextTask();
	BOOST_CHECK(state == 6);

	scheduler.ExecNextTask();
	BOOST_CHECK(state == 10);
}

BOOST_AUTO_TEST_CASE( test_scheduler_exec_next_task )
{
	scheduler scheduler;
	size_t state(0);
	scheduler.AddTask([&state]() { state += 1; });
	scheduler.AddTask([&state]() { state += 2; });

	BOOST_CHECK(scheduler.TaskCount() == 2);
	BOOST_CHECK(state == 0);

	BOOST_CHECK(scheduler.ExecNextTask() == true);

	BOOST_CHECK(scheduler.TaskCount() == 1);
	BOOST_CHECK(state == 1);

	BOOST_CHECK(scheduler.ExecNextTask() == true);

	BOOST_CHECK(scheduler.TaskCount() == 0);
	BOOST_CHECK(state == 3);

	BOOST_CHECK(scheduler.ExecNextTask() == false);

	BOOST_CHECK(scheduler.TaskCount() == 0);
	BOOST_CHECK(state == 3);
}

BOOST_AUTO_TEST_CASE( test_scheduler_exec_all_tasks )
{
	scheduler scheduler;
	size_t state(0);
	scheduler.AddTask([&state]() { state += 1; });
	scheduler.AddTask([&state]() { state += 2; });

	BOOST_CHECK(scheduler.TaskCount() == 2);
	BOOST_CHECK(state == 0);

	BOOST_CHECK(scheduler.ExecAllTasks() == 2);
	BOOST_CHECK(scheduler.TaskCount() == 0);
	BOOST_CHECK(state == 3);
}

//
// TestScheduler.cpp ends here
