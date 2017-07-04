/*
 * test_parallelscheduler.cpp
 *
 *  Created on: Sep 24, 2015
 *      Author: ckielwein
 */

#include <flexcore/scheduler/cyclecontrol.hpp>
#include <flexcore/scheduler/parallelscheduler.hpp>
#include <boost/test/unit_test.hpp>

#include <functional>

#include <iostream>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_scheduler)

namespace
{
std::unique_ptr<thread::cycle_control> make_parallel_cycle_control()
{
	return std::make_unique<thread::cycle_control>(std::make_unique<thread::parallel_scheduler>());
}

struct store
{
	int val = 0;

	void make_1()
	{
		val = 1;
	}
};
}
BOOST_AUTO_TEST_CASE(test_single_execution)
{
	store test_values{};
	{
	auto test_scheduler = make_parallel_cycle_control();

	{
		thread::periodic_task task1(std::bind(&store::make_1, &test_values));
		test_scheduler->add_task(std::move(task1), thread::cycle_control::fast_tick);
	}

	test_scheduler->work();

	while (test_scheduler->nr_of_tasks() != 0)
	{
		//do nothing but wait
	}
	BOOST_CHECK_EQUAL(test_scheduler->nr_of_tasks(), 0);

	}

	BOOST_CHECK_EQUAL(test_values.val, 1);

	{
		auto test_scheduler = make_parallel_cycle_control();

	store test_values_2{};
	{
		thread::periodic_task task_2(std::bind(&store::make_1, &test_values_2));
		test_scheduler->add_task(std::move(task_2), thread::cycle_control::fast_tick);
	}

	BOOST_CHECK_EQUAL(test_values_2.val, 0);
	test_scheduler->work();
	while (test_scheduler->nr_of_tasks() != 0)
	{
		//do nothing but wait
	}
	test_scheduler->stop();
	BOOST_CHECK_EQUAL(test_values_2.val, 1);
	}
}

BOOST_AUTO_TEST_CASE(test_multiple_execution)
{
	constexpr int nr_of_tasks{20};
	std::vector<store> test_values(nr_of_tasks);
	{
	auto test_scheduler = make_parallel_cycle_control();

	for(auto i = begin(test_values); i != end(test_values); ++i)
	{
			thread::periodic_task task(std::bind(&store::make_1, &(*i)));
			test_scheduler->add_task(std::move(task), thread::cycle_control::fast_tick);
	}

	test_scheduler->work();
	while (test_scheduler->nr_of_tasks() != 0)
	{
		//do nothing but wait
	}
	BOOST_CHECK_EQUAL(test_scheduler->nr_of_tasks(), 0);	}
	for(auto& single_task: test_values)
	{
		BOOST_CHECK_EQUAL(single_task.val, 1);
	}
}

BOOST_AUTO_TEST_CASE(test_main_loop)
{

	store test_values{};
	auto test_scheduler = make_parallel_cycle_control();
	{
		thread::periodic_task task1(std::bind(&store::make_1, &test_values));
		test_scheduler->add_task(std::move(task1), thread::cycle_control::fast_tick);
	}
	test_scheduler->start(); //start main loop
	sleep(1); //todo remove this hack,
	//currently needed because this function runs through
	//while the task is being added to the working threads by the scheduler
	while (test_scheduler->nr_of_tasks() != 0)
	{
		//do nothing but wait
	}
	test_scheduler->stop(); // stop main loop
	BOOST_CHECK_EQUAL(test_values.val, 1);

}

BOOST_AUTO_TEST_SUITE_END()
