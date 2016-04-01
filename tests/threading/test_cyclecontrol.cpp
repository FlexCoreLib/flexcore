/*
 * test_cyclecontrol.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: jschwan
 */

// boost
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <unistd.h>

#include <threading/cyclecontrol.hpp>
#include <threading/parallelscheduler.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_cyclecontrol)

BOOST_AUTO_TEST_CASE( test_cyclecontrol_task_not_finished_in_time)
{
	fc::thread::cycle_control thread_manager{std::make_unique<thread::parallel_scheduler>()};
	std::atomic<bool> terminate_thread(false);
	{
		auto tick_cycle = fc::thread::periodic_task(
		    [&terminate_thread]()
		    {
			    while (!terminate_thread)
				    usleep(100);
		    });

		thread_manager.add_task(std::move(tick_cycle), fc::thread::cycle_control::fast_tick);
	}
	thread_manager.start();

	bool exception_thrown = false;
	for (int i = 0; i < 300; ++i)
	{
		if (auto except_ptr = thread_manager.last_exception())
		{
			BOOST_CHECK_THROW(std::rethrow_exception(except_ptr), std::runtime_error);
			exception_thrown = true;
			terminate_thread = true;
			break;
		}
		usleep(10000);
	}

	BOOST_CHECK(exception_thrown);
	BOOST_CHECK(terminate_thread);
}

BOOST_AUTO_TEST_CASE(test_adding_tasks_to_running_scheduler)
{
	namespace sched = fc::thread;
	sched::cycle_control controller{std::make_unique<sched::parallel_scheduler>()};
	controller.start();
	BOOST_CHECK_THROW(controller.add_task({[]{}}, sched::cycle_control::fast_tick), std::runtime_error);
	controller.stop();
	BOOST_CHECK_THROW(controller.add_task({[]{}}, 2 * sched::cycle_control::slow_tick), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_fast_main_loop)
{
	namespace sched = fc::thread;
	using cycle = sched::cycle_control;
	sched::cycle_control controller{std::make_unique<sched::parallel_scheduler>()};
	auto count_fast = 0ull;
	auto count_medium = 0ull;
	auto count_slow = 0ull;
	controller.add_task({[&] { ++count_fast; }}, cycle::fast_tick);
	controller.add_task({[&] { ++count_medium; }}, cycle::medium_tick);
	controller.add_task({[&] { ++count_slow; }}, cycle::slow_tick);
	controller.start(true);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	controller.stop();
	auto ratio_fast_medium = static_cast<double>(count_fast) / count_medium;
	auto ratio_medium_slow = static_cast<double>(count_medium) / count_slow;
	auto ratio_fast_slow = static_cast<double>(count_fast) / count_slow;
	BOOST_CHECK_CLOSE_FRACTION(ratio_fast_medium, 10.0, 1);
	BOOST_CHECK_CLOSE_FRACTION(ratio_medium_slow, 10.0, 1);
	BOOST_CHECK_CLOSE_FRACTION(ratio_fast_slow, 100.0, 1);
	BOOST_TEST_MESSAGE("Fast count: " << count_fast);
	BOOST_TEST_MESSAGE("Medium count: " << count_medium);
	BOOST_TEST_MESSAGE("Slow count: " << count_slow);
}
BOOST_AUTO_TEST_SUITE_END()
