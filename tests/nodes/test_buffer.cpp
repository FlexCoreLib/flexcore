#include <boost/test/unit_test.hpp>

#include <nodes/buffer.hpp>
#include <ports/events/event_sources.hpp>
#include <ports/states/state_sink.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_buffers)

BOOST_AUTO_TEST_CASE(single_event_to_state)
{
	list_collector<int, swap_on_tick> buffer;

	event_out_port<int> source;

	source >> buffer.in();

	BOOST_CHECK(buffer.out()().empty());

	source.fire(1);
	BOOST_CHECK(buffer.out()().empty());

	buffer.swap_buffers()();
	BOOST_CHECK(!buffer.out()().empty());
	BOOST_CHECK_EQUAL(buffer.out()().front(), 1);

	buffer.swap_buffers()();
	BOOST_CHECK(buffer.out()().empty());

	buffer.swap_buffers()();
	buffer.swap_buffers()();
	BOOST_CHECK(buffer.out()().empty());
}

BOOST_AUTO_TEST_CASE(event_range_to_state)
{
	list_collector<int, swap_on_tick> buffer;
	typedef boost::iterator_range<std::vector<int>::iterator> int_range;
	event_out_port<int_range> source;
	std::vector<int> vec {1,2,3,4};

	source >> buffer.in();

	BOOST_CHECK(buffer.out()().empty());

	source.fire(int_range(vec.begin(), vec.end()));
	BOOST_CHECK(buffer.out()().empty());

	buffer.swap_buffers()();
	BOOST_CHECK(!buffer.out()().empty());
	BOOST_CHECK(buffer.out()().size() == static_cast<int>(vec.size()));

	source.fire(int_range(vec.begin(), vec.end()));
	source.fire(int_range(vec.begin(), vec.end()));
	buffer.swap_buffers()();
	BOOST_CHECK(buffer.out()().size() == static_cast<int>(vec.size())*2);

}

BOOST_AUTO_TEST_CASE(test_hold_last)
{
	hold_last<int> buffer;

	event_out_port<int> source;
	state_sink<int> sink;

	source >> buffer.in();
	buffer.out() >> sink;
	BOOST_CHECK_EQUAL(sink.get(), 0);

	source.fire(1);
	BOOST_CHECK_EQUAL(sink.get(), 1);
}

BOOST_AUTO_TEST_CASE(test_hold_n)
{
	hold_n<int> buffer{3};
	event_out_port<int> source;
	state_sink<hold_n<int>::out_range_t> sink;

	source >> buffer.in();
	buffer.out() >> sink;
	BOOST_CHECK(sink.get().empty());

	source.fire(1);
	BOOST_CHECK_EQUAL(sink.get().size(), 1);

	source.fire(1);
	BOOST_CHECK_EQUAL(sink.get().size(), 2);
	source.fire(1);
	BOOST_CHECK_EQUAL(sink.get().size(), 3); //capacity reached

	source.fire(1);
	BOOST_CHECK_EQUAL(sink.get().size(), 3); //capacity reached
}

BOOST_AUTO_TEST_CASE(test_hold_n_incoming_range)
{
	hold_n<int> buffer{5};
	state_sink<hold_n<int>::out_range_t> sink;
	typedef boost::iterator_range<std::vector<int>::iterator> int_range;
	event_out_port<int_range> source;
	std::vector<int> vec {1,2,3,4};

	source >> buffer.in();
	buffer.out() >> sink;
	BOOST_CHECK(sink.get().empty());

	source.fire(int_range(vec.begin(), vec.end()));
	//size is smaller then capacity
	BOOST_CHECK_EQUAL(sink.get().size(), vec.size());

	source.fire(int_range(vec.begin(), vec.end()));
	BOOST_CHECK_EQUAL(sink.get().size(), 5); //capacity reached
	source.fire(int_range(vec.begin(), vec.end()));
	BOOST_CHECK_EQUAL(sink.get().size(), 5); //capacity reached
}

BOOST_AUTO_TEST_SUITE_END()
