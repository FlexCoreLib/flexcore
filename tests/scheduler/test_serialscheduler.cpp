#include <boost/test/unit_test.hpp>

#include <scheduler/serialschedulers.hpp>
#include <future>

auto make_blocking_scheduler()
{
	return std::make_unique<fc::thread::blocking_scheduler>();
}

BOOST_AUTO_TEST_SUITE(test_blocking_scheduler)
BOOST_AUTO_TEST_CASE(test_can_add_tasks)
{
	auto scheduler = make_blocking_scheduler();
	auto number = 0;
	BOOST_CHECK_EQUAL(number, 0);
	scheduler->add_task([&] { number = 1; });
	BOOST_CHECK_EQUAL(number, 1);
}

BOOST_AUTO_TEST_CASE(test_cant_add_tasks_after_stopped)
{
	auto scheduler = make_blocking_scheduler();
	scheduler->stop();
	BOOST_CHECK_THROW(scheduler->add_task([]{}), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_nr_of_waiting_tasks)
{
	auto scheduler = make_blocking_scheduler();
	std::promise<void> task_is_running;
	std::promise<void> terminate_task;
	auto terminate = terminate_task.get_future();
	auto running = task_is_running.get_future();
	auto check_running_task = std::async(std::launch::async, [&]
	                                     {
		                                     running.get();
		                                     BOOST_CHECK_EQUAL(scheduler->nr_of_waiting_tasks(), 1);
		                                     terminate_task.set_value();
	                                     });
	BOOST_CHECK_EQUAL(scheduler->nr_of_waiting_tasks(), 0);
	scheduler->add_task([&]
	                    {
		                    task_is_running.set_value();
		                    terminate.get();
	                    });
}
BOOST_AUTO_TEST_SUITE_END()

