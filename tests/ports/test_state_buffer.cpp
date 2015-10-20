// boost
#include <boost/test/unit_test.hpp>

#include <ports/ports.hpp>
#include <ports/state_ports.hpp>
#include <ports/statebuffer.hpp>

using namespace fc;

BOOST_AUTO_TEST_CASE( test_state_buffer )
{
	state_buffer<int> test_buffer;
	state_source_with_setter<int> source(1);
	state_sink<int> sink;

	source >> test_buffer.in();
	test_buffer.out() >> sink;

	BOOST_CHECK_EQUAL(sink.get(), 0);

	test_buffer.work_tick()();
	test_buffer.switch_tick()();

	BOOST_CHECK_EQUAL(sink.get(), 1);
	// sanity check, that we can call the buffer multiple times
	// and still get the same result.
	BOOST_CHECK_EQUAL(sink.get(), 1);

	source.set(2);
	//since work wasn't ticked since the last switch, expect no change
	test_buffer.switch_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 1);

	test_buffer.work_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 1);
	test_buffer.switch_tick()();
	BOOST_CHECK_EQUAL(sink.get(), 2);
}
