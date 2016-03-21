/*
 * test_cyclecontrol.cpp
 *
 *  Created on: Nov 25, 2015
 *      Author: jschwan
 */

// boost
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <unistd.h>

#include <threading/cyclecontrol.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_cyclecontrol)

BOOST_AUTO_TEST_CASE( test_cyclecontrol_task_not_finished_in_time)
{
	fc::thread::cycle_control thread_manager;
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

BOOST_AUTO_TEST_SUITE_END()
