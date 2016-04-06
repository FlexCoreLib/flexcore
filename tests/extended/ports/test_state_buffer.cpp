// boost
#include <boost/test/unit_test.hpp>

#include <flexcore/extended/ports/connection_buffer.hpp>
#include <flexcore/pure/pure_ports.hpp>


using namespace fc;

BOOST_AUTO_TEST_CASE( test_state_buffer )
{
	state_buffer<int> test_buffer;
	int test_state = 1;
	pure::state_source<int> source([&test_state](){ return test_state; });
	pure::state_sink<int> sink;

	source >> test_buffer.in();
	test_buffer.out() >> sink;

	BOOST_CHECK_EQUAL(sink.get(), 0);

	// writes to incoming buffer
	test_buffer.work_tick()();
	// only moves to middle buffer, doesn't make accessible to outgoing buffer
	test_buffer.switch_passive_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 0);

	// moves middle buffer content to outgoing ubffer
	test_buffer.switch_active_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 1);
	// sanity check, that we can call the buffer multiple times
	// and still get the same result.
	BOOST_CHECK_EQUAL(sink.get(), 1);

	test_state = 2;
	//since work wasn't ticked since the last switch, expect no change
	test_buffer.switch_passive_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 1);
	// ditto
	test_buffer.switch_active_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 1);

	test_buffer.work_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 1);
	test_buffer.switch_passive_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 1);
	test_buffer.switch_active_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 2);
}
