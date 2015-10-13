/*
 * test_parallelscheduler.cpp
 *
 *  Created on: Sep 24, 2015
 *      Author: ckielwein
 */

#include <boost/test/unit_test.hpp>

#include <threading/parallelscheduler.hpp>
#include <threading/cyclecontrol.hpp>

#include <functional>

#include <iostream>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_scheduler)

struct store
{
	int val = 0;

	void make_1()
	{
		val = 1;
	}

	void make_2()
	{
		val = 2;
	}
};

BOOST_AUTO_TEST_CASE(test_single_execution)
{
	store test_values;
	{
	thread::cycle_control test_scheduler;

	thread::periodic_task task1(std::bind(&store::make_1, &test_values),
			chrono::virtual_clock::duration::min());

	test_scheduler.add_task(task1);

	test_scheduler.work();

	while (test_scheduler.nr_of_tasks() != 0)
	{
		//do nothing but wait
	}
	BOOST_CHECK_EQUAL(test_scheduler.nr_of_tasks(), 0);

	}

	BOOST_CHECK_EQUAL(test_values.val, 1);

}

BOOST_AUTO_TEST_CASE(test_multiple_execution)
{
	const int nr_of_tasks = 20;
	std::vector<store> test_values(nr_of_tasks);
	{
	thread::cycle_control test_scheduler;

	for(auto i = begin(test_values); i != end(test_values); ++i)
	{
			thread::periodic_task task (std::bind(&store::make_1, &(*i)),
					chrono::virtual_clock::duration::min());

			test_scheduler.add_task(task);
	}

	test_scheduler.work();
	while (test_scheduler.nr_of_tasks() != 0)
	{
		//do nothing but wait
	}
	BOOST_CHECK_EQUAL(test_scheduler.nr_of_tasks(), 0);	}
	for(auto& single_task: test_values)
	{
		BOOST_CHECK_EQUAL(single_task.val, 1);
	}
}

BOOST_AUTO_TEST_CASE(test_main_loop)
{

	store test_values;
	thread::cycle_control test_scheduler;
	thread::periodic_task task1(std::bind(&store::make_1, &test_values),
			chrono::virtual_clock::duration::min());

std::cout << "start main_loop" << std::endl;

	test_scheduler.add_task(task1);
	test_scheduler.start(); //start main loop
	sleep(1);
	while (test_scheduler.nr_of_tasks() != 0)
	{
		//do nothing but wait
	}
	test_scheduler.stop(); // stop main loop
	BOOST_CHECK_EQUAL(test_values.val, 1);

}

BOOST_AUTO_TEST_SUITE_END()

