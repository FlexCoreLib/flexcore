#include <boost/test/unit_test.hpp>

#include <nodes/buffer.hpp>
#include <ports/events/event_sources.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_buffers)

BOOST_AUTO_TEST_CASE(single_event_to_state)
{
	event_to_state<int> buffer;

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
	event_to_state<int> buffer;
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

BOOST_AUTO_TEST_SUITE_END()
