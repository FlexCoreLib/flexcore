#include <boost/test/unit_test.hpp>

#include <flexcore/scheduler/serialschedulers.hpp>
#include <future>

namespace
{
auto make_blocking_scheduler()
{
	return std::make_unique<fc::thread::blocking_scheduler>();
}
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
	std::atomic_bool ran{false};
	std::atomic_bool finish{false};
	bool nr_was_one = false;
	auto check = std::thread([&]
	                        {
		                        while (!ran.load())
			                        std::this_thread::yield();
		                        nr_was_one = scheduler->nr_of_waiting_tasks() == 1;
		                        finish.store(true);
	                        });
	BOOST_CHECK_EQUAL(scheduler->nr_of_waiting_tasks(), 0);
	scheduler->add_task([&]
	                    {
		                    ran.store(true);
		                    while (!finish.load())
			                    std::this_thread::yield();
	                    });
	check.join();
	BOOST_CHECK(nr_was_one);
}
BOOST_AUTO_TEST_SUITE_END()

