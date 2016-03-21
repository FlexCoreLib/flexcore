#include <boost/test/unit_test.hpp>
#include <ports/pure_ports.hpp>
#include <ports/mux_ports.hpp>
#include <core/connection.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(mux_ports)

struct mux_fixture
{
	pure::state_source<int> a{[] {return 1;}};
	pure::state_source<int> b{[] {return 2;}};
	pure::state_source<int> c{[] {return 3;}};
	pure::state_sink<int> sink_a, sink_b, sink_c;

	decltype(mux(a, b, c)) muxed_sources = mux(a, b, c);
	decltype(mux(sink_a, sink_b, sink_c)) muxed_sinks = mux(sink_a, sink_b, sink_c);
};

BOOST_FIXTURE_TEST_CASE(mux_to_mux_connection, mux_fixture)
{
	muxed_sources >> muxed_sinks;
	BOOST_CHECK_EQUAL(sink_a.get(), 1);
	BOOST_CHECK_EQUAL(sink_b.get(), 2);
	BOOST_CHECK_EQUAL(sink_c.get(), 3);
}

BOOST_FIXTURE_TEST_CASE(mux_to_mux_with_lambda, mux_fixture)
{
	muxed_sources >> [] (int i) { return -i; } >> muxed_sinks;
	BOOST_CHECK_EQUAL(sink_a.get(), -1);
	BOOST_CHECK_EQUAL(sink_b.get(), -2);
	BOOST_CHECK_EQUAL(sink_c.get(), -3);
}

BOOST_FIXTURE_TEST_CASE(mux_merge_sink, mux_fixture)
{
	pure::state_sink<int> sink;
	auto negate = [](auto x) { return -x; };
	muxed_sources
	    >> negate
	    >> merge([](auto a, auto b, auto c) { return a + b + c; })
	    >> negate
	    >> sink;
	BOOST_CHECK_EQUAL(sink.get(), 6);
}
BOOST_AUTO_TEST_SUITE_END()
