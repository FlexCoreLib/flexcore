#include <boost/test/unit_test.hpp>

#include <ports/event_buffer.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_eventbuffer);

BOOST_AUTO_TEST_CASE(test_event_buffer)
{
	event_buffer<int> test_buffer;

	int test_value = 0;
	auto write_param = [&](int i) {test_value = i;};
	event_in_port<int> sink(write_param);
	event_out_port<int> source;

	source >> test_buffer.in_events();
	test_buffer.out_events() >> sink;

	source.fire(1);
	BOOST_CHECK_EQUAL(test_value, 0);
	test_buffer.switch_tick()();
	BOOST_CHECK_EQUAL(test_value, 0);
	test_buffer.send_tick()();
	BOOST_CHECK_EQUAL(test_value, 1);

	//repeat process to check for buffer overflows
	for (int i = 2; i!= 1000; ++i)
	{
		source.fire(i);
		BOOST_CHECK_EQUAL(test_value, i-1);
		test_buffer.switch_tick()();
		BOOST_CHECK_EQUAL(test_value, i-1);
		test_buffer.send_tick()();
		BOOST_CHECK_EQUAL(test_value, i);
	}
}

BOOST_AUTO_TEST_SUITE_END();
