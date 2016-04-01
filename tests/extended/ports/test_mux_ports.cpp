#include <boost/test/unit_test.hpp>
#include <pure/pure_ports.hpp>
#include <pure/mux_ports.hpp>
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
	std::move(muxed_sources) >> std::move(muxed_sinks);
	BOOST_CHECK_EQUAL(sink_a.get(), 1);
	BOOST_CHECK_EQUAL(sink_b.get(), 2);
	BOOST_CHECK_EQUAL(sink_c.get(), 3);
}

BOOST_FIXTURE_TEST_CASE(mux_to_mux_with_lambda, mux_fixture)
{
	std::move(muxed_sources) >> [] (int i) { return -i; } >> std::move(muxed_sinks);
	BOOST_CHECK_EQUAL(sink_a.get(), -1);
	BOOST_CHECK_EQUAL(sink_b.get(), -2);
	BOOST_CHECK_EQUAL(sink_c.get(), -3);
}

BOOST_FIXTURE_TEST_CASE(mux_merge_sink, mux_fixture)
{
	pure::state_sink<int> sink;
	auto negate = [](auto x) { return -x; };
	std::move(muxed_sources)
	    >> negate
	    >> merge([](auto a, auto b, auto c) { return a + b + c; })
	    >> negate
	    >> sink;
	BOOST_CHECK_EQUAL(sink.get(), 6);
}

BOOST_AUTO_TEST_CASE(event_src_mux_to_sink)
{
	int counter = 0;
	pure::event_source<int> a, b;
	pure::event_sink<int> sink{[&] (int i) { counter += i; }};
	mux(a,
	    b) >> sink;
	BOOST_CHECK_EQUAL(counter, 0);
	a.fire(3);
	BOOST_CHECK_EQUAL(counter, 3);
	b.fire(-4);
	BOOST_CHECK_EQUAL(counter, -1);
}

BOOST_AUTO_TEST_CASE(event_src_to_mux_sink)
{
	const int fire_val = 12345;
	pure::event_source<int> src;
	pure::event_sink<int> sink_a{[=](int i) { BOOST_CHECK_EQUAL(fire_val, i); }};
	pure::event_sink<int> sink_b{[=](int i) { BOOST_CHECK_EQUAL(fire_val, i); }};
	src >> mux(sink_a,
	           sink_b);
	src.fire(fire_val);
}
BOOST_AUTO_TEST_SUITE_END()
