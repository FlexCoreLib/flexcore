#include <boost/test/unit_test.hpp>

#include "threading/parallelregion.hpp"

using namespace fc;

BOOST_AUTO_TEST_CASE( test_ownership_handling)
{
	struct dummy_region : parallel_region
	{
		dummy_region(std::shared_ptr<tick_controller> tick) :
			parallel_region(tick)
		{
			get_ticks()->switch_tick() >> [this](){ switch_count = switch_count+1;};
			get_ticks()->fire_outgoing() >> [this](){ fire_count = fire_count+1;};
			get_ticks()->work_tick() >> [this](){ work_count = work_count+1;};
		}

		int switch_count = 0;
		int fire_count = 0;
		int work_count = 0;
	};

	dummy_region test_region_1(std::make_shared<tick_controller>());
	dummy_region test_child(test_region_1.get_ticks());

	test_region_1.get_ticks()->in_switch_buffers()();
	// switch tick is forwarded to child buffers, thus their count should increase
	BOOST_CHECK_EQUAL(test_region_1.switch_count, 1);
	BOOST_CHECK_EQUAL(test_child.switch_count, 1);

	test_region_1.get_ticks()->in_fire_outgoing()();
	// fire tick is not forwarded
	BOOST_CHECK_EQUAL(test_region_1.fire_count, 1);
	BOOST_CHECK_EQUAL(test_child.fire_count, 0);

	test_region_1.get_ticks()->in_work()();
	// work tick is not forwarded
	BOOST_CHECK_EQUAL(test_region_1.work_count, 1);
	BOOST_CHECK_EQUAL(test_child.work_count, 0);
	//work tick fires outgoing events of child region
	BOOST_CHECK_EQUAL(test_child.fire_count, 1);

	dummy_region test_child_2(test_region_1.get_ticks());
	test_region_1.get_ticks()->in_switch_buffers()();
	// switch tick is forwarded to child buffers, thus their count should increase
	BOOST_CHECK_EQUAL(test_region_1.switch_count, 2);
	BOOST_CHECK_EQUAL(test_child.switch_count, 2);
	BOOST_CHECK_EQUAL(test_child_2.switch_count, 1);
}

BOOST_AUTO_TEST_CASE( test_region_buffers)
{
	struct test_region_t : parallel_region
	{
		test_region_t(std::shared_ptr<tick_controller> tick) :
			parallel_region(tick)
		{
			attach_event_in_buffer(incoming_event); // ToDo move this to the bufffer, which only gets this pointer in constructor
			attach_event_out_buffer(leaving_event);

			incoming_event >> [this](int i) {storage = i; };
			incoming_event >> [](int i) { return ++i; } >> leaving_event;
		}

		exit_event<int> leaving_event;
		enter_event<int> incoming_event;

		int storage = 0;
	};

	test_region_t test_region(std::make_shared<tick_controller>());
	event_out_port<int> sender;
	std::vector<int> sink_buffer;
	event_in_port<int> sink([&](int i){ sink_buffer.push_back(i); });

	sender >> test_region.incoming_event.region_port();
	test_region.leaving_event.region_port() >> sink;
	sender.fire(1);

	// since the buffer was not yet switched, the event has no effect yet.
	BOOST_CHECK_EQUAL(test_region.storage, 0);

	test_region.get_ticks()->in_switch_buffers()();
	// since the region has not yet had its work tick triggered...
	BOOST_CHECK_EQUAL(test_region.storage, 0);

	test_region.get_ticks()->in_work()();
	BOOST_CHECK_EQUAL(test_region.storage, 1); //now!

	test_region.get_ticks()->in_switch_buffers()();
	BOOST_CHECK_EQUAL(sink_buffer.size(), 0);

	test_region.get_ticks()->in_fire_outgoing()();
	BOOST_CHECK_EQUAL(sink_buffer.size(), 1);
	BOOST_CHECK_EQUAL(sink_buffer.front(), 2);
}

BOOST_AUTO_TEST_CASE( test_region_state)
{

}

