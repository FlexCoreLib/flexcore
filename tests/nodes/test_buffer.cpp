#include <boost/test/unit_test.hpp>

#include <flexcore/extended/nodes/buffer.hpp>
#include <flexcore/extended/base_node.hpp>
#include <flexcore/pure/event_sources.hpp>
#include <flexcore/pure/state_sink.hpp>

#include "owning_node.hpp"

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_buffers)

using collector_t = list_collector<int, swap_on_tick, tree_base_node>;

BOOST_AUTO_TEST_CASE(single_event_to_state)
{
	tests::owning_node root;
	auto buffer = root.make_child_named<collector_t>("collector");
	event_source<int> source{&root.node()};

	source >> buffer->in();

	BOOST_CHECK(buffer->out()().empty());

	source.fire(1);
	BOOST_CHECK(buffer->out()().empty());

	buffer->swap_buffers()();
	BOOST_CHECK(!buffer->out()().empty());
	BOOST_CHECK_EQUAL(buffer->out()().front(), 1);

	buffer->swap_buffers()();
	BOOST_CHECK(buffer->out()().empty());

	buffer->swap_buffers()();
	buffer->swap_buffers()();
	BOOST_CHECK(buffer->out()().empty());
}

BOOST_AUTO_TEST_CASE(event_range_to_state)
{
	tests::owning_node root;

	auto buffer = root.make_child_named<collector_t>("collector");
	typedef boost::iterator_range<std::vector<int>::iterator> int_range;
	event_source<int_range> source{&root.node()};
	std::vector<int> vec {1,2,3,4};

	source >> buffer->in();

	BOOST_CHECK(buffer->out()().empty());

	source.fire(int_range(vec.begin(), vec.end()));
	BOOST_CHECK(buffer->out()().empty());

	buffer->swap_buffers()();
	BOOST_CHECK(!buffer->out()().empty());
	BOOST_CHECK(buffer->out()().size() == static_cast<int>(vec.size()));

	source.fire(int_range(vec.begin(), vec.end()));
	source.fire(int_range(vec.begin(), vec.end()));
	buffer->swap_buffers()();
	BOOST_CHECK(buffer->out()().size() == static_cast<int>(vec.size())*2);

}

BOOST_AUTO_TEST_CASE(test_hold_last)
{
	tests::owning_node root;

	auto buffer = root.make_child<hold_last<int, tree_base_node>>(0);

	event_source<int> source{&root.node()};
	state_sink<int> sink{&root.node()};

	source >> buffer->in();
	buffer->out() >> sink;
	BOOST_CHECK_EQUAL(sink.get(), 0);

	source.fire(1);
	BOOST_CHECK_EQUAL(sink.get(), 1);
}

BOOST_AUTO_TEST_CASE(test_hold_n)
{
	tests::owning_node root;

	auto buffer = root.make_child<hold_n<int, tree_base_node>>(3);

	event_source<int> source{&root.node()};
	state_sink<hold_n<int, tree_base_node>::out_range_t> sink{&root.node()};

	source >> buffer->in();
	buffer->out() >> sink;
	BOOST_CHECK(sink.get().empty());

	source.fire(0);
	BOOST_CHECK_EQUAL(sink.get().size(), 1);
	BOOST_CHECK_EQUAL(sink.get().front(), 0);
	BOOST_CHECK_EQUAL(sink.get().back(), 0);


	source.fire(1);
	BOOST_CHECK_EQUAL(sink.get().size(), 2);
	BOOST_CHECK_EQUAL(sink.get().back(), 1);
	BOOST_CHECK_EQUAL(sink.get().front(), 0); // first event is still in buffer

	source.fire(1);
	BOOST_CHECK_EQUAL(sink.get().size(), 3); // capacity reached

	source.fire(2);
	BOOST_CHECK_EQUAL(sink.get().size(), 3); // capacity reached
	// first event has been pushed out of buffer as capacity was reached before
	BOOST_CHECK_EQUAL(sink.get().back(), 2);
	BOOST_CHECK_EQUAL(sink.get().front(), 1); // not 0 as before

}

BOOST_AUTO_TEST_CASE(test_hold_n_incoming_range)
{
	tests::owning_node root;

	auto buffer = root.make_child<hold_n<int, tree_base_node>>(5);

	state_sink<hold_n<int, tree_base_node>::out_range_t> sink{&root.node()};
	typedef boost::iterator_range<std::vector<int>::iterator> int_range;
	event_source<int_range> source{&root.node()};
	std::vector<int> vec {1,2,3,4};

	source >> buffer->in();
	buffer->out() >> sink;
	BOOST_CHECK(sink.get().empty());

	source.fire(int_range(vec.begin(), vec.end()));
	//size is smaller then capacity
	BOOST_CHECK_EQUAL(sink.get().size(), vec.size());
	BOOST_CHECK(sink.get() == vec);

	source.fire(int_range(vec.begin(), vec.end()));

	BOOST_CHECK_EQUAL(sink.get().size(), 5); // capacity reached
	source.fire(int_range(vec.begin(), vec.end()));
	BOOST_CHECK_EQUAL(sink.get().size(), 5); // capacity reached
	// last element remaining from previous range is the back of the vector
	BOOST_CHECK_EQUAL(sink.get().front(), vec.back());
	BOOST_CHECK_EQUAL(sink.get().back(), vec.back());
}

BOOST_AUTO_TEST_SUITE_END()
