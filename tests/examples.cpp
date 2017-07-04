#include <boost/test/unit_test.hpp>

///\example examples.cpp
//This header includes the default definitions of ports
#include <flexcore/ports.hpp>

// This Header includes the code for multiplexing streams.
#include <flexcore/pure/mux_ports.hpp>
// This header includes several useful connectables.
#include <flexcore/core/connectables.hpp>
// This header includes the algorithms which work on ranges.
#include <flexcore/range/actions.hpp>

#include <flexcore/extended/nodes/terminal.hpp>
#include <flexcore/extended/nodes/buffer.hpp>
#include <flexcore/pure/pure_node.hpp>

// We need to pull operator >> of flexcore to our local namespace.
// Otherwise the operator cannot be found if we connect elements,
// which are not from the flexcore namespace.
using fc::operator>>;

BOOST_AUTO_TEST_SUITE(examples)

// This test shows how to send events between nodes in flexcore.
BOOST_AUTO_TEST_CASE(events_example)
{
	// in this example we don't use the full extended infrastructure.
	// Nodes and ports from the pure namespace have no notion of parallel region.
	// Ownership of these nodes can and has to be managed manually.

	// Terminal nodes just forward all data and do no computations at all.
	fc::event_terminal<int, fc::pure::pure_node> source{};

	//Hold last stores the last event received and provides it as state.
	fc::hold_last<int, fc::pure::pure_node> sink{0};

	//connections can be chained
	// with as many lambdas or other connectables in between as we want.
	source.out()
			>> fc::increment{}
			>> [](int i){ return i * 10; }
			>> sink.in();

	// we can call event sinks with their operator() to send inputs.
	source.in()(0);
	BOOST_CHECK_EQUAL(sink.out()(), 10);

	fc::event_terminal<int, fc::pure::pure_node> source_2{};

	// an event_sink can be connected to more than one source
	// it will receive tokens from all sources, when they fire events.
	source_2.out() >> sink.in();
	source.in()(42);
	BOOST_CHECK_EQUAL(sink.out()(), (42+1)*10);
	source_2.in()(42);
	BOOST_CHECK_EQUAL(sink.out()(), 42);

	fc::hold_last<int, fc::pure::pure_node> sink_2{0};
	// an event source can be connected to more than one sink
	// it will send tokens to all sinks, when it fires events.
	source_2.out() >> sink_2.in();

	source_2.in()(666);
	BOOST_CHECK_EQUAL(sink.out()(), 666);
	BOOST_CHECK_EQUAL(sink_2.out()(), 666);
}


// This test shows how to transmit states between nodes.
// See tests/nodes/test_state_nodes for uses of several nodes which operate on states.
BOOST_AUTO_TEST_CASE(state_example)
{
	// in this example we don't use the full extended infrastructure.
	// Nodes and ports from the pure namespace have no notion of parallel region.
	// Ownership of these nodes can and has to be managed manually.
	fc::state_terminal<int, fc::pure::pure_node> source{};
	fc::state_terminal<int, fc::pure::pure_node> sink{};

	//provide a constant state as inputs to our node
	fc::constant(42) >> source.in();

	// this simple chain increments all data from our source
	// and makes them available to our sink
	source.out() >> fc::increment{} >> sink.in();

	//every time we pull data from our sink we get the result from the chain.
	BOOST_CHECK_EQUAL(sink.out()(), 42 +1);

	fc::state_terminal<int, fc::pure::pure_node> sink_2{};
	// a state source an be connected to more than one state sink
	// each sink is able to pull the tokens separately.
	// Calculations within the chain might be done twice.
	source.out() >> sink_2.in();
	BOOST_CHECK_EQUAL(sink_2.out()(), 42);

	// Every State sink can only pull from a single source.
	// this the following line would overwrite the previous connection to source
	//fc::constant(2) >> source.in()
}

// This test shows the mechanics flexcore provides to operator on ranges and lists of values.
BOOST_AUTO_TEST_CASE(range_example)
{
	std::vector<int> vec {-4, -3, -2, -1, 0, 1, 2, 3, 4};

	fc::pure::state_source<std::vector<int>> source(fc::constant(vec));
	fc::pure::state_sink<int> sink{};

	source
	// This filters the source range and only lets the elements smaller than zero through
			>> fc::actions::filter([](int i){ return i < 0;})
	// This multiplies every element in the filtered range by two
			>> fc::actions::map([](int i){ return i*2;})
	// this sums all elements up and starts with zero.
			>> fc::sum(0)
			>> sink;

	BOOST_CHECK_EQUAL(sink.get(), -20);
}

// This test shows how to use the mux mechanic to combine streams.
// See tests/extended/ports/test/mux_ports.cpp for more uses of mux.
BOOST_AUTO_TEST_CASE(mux_example)
{
	//use the shortcut for a passive source, which provides a constant value.
	const auto source_1 = fc::constant(1.234);
	const auto source_2 = fc::constant(5.678);

	fc::pure::state_sink<double> sink{};

	//mux creates a multiplexed connectable from the two sources.
	fc::mux(source_1, source_2)
		// connectables in a multiplexed connection
		// are applied to all of the multiplexed streams.
		>> [](double in){ return in * -1;}
		// this merges the multiplex streams to one stream
		// in this case it adds the values from the to streams
		>> fc::merge([](auto a, auto b) { return a + b; })
		>> sink;

	BOOST_CHECK_EQUAL(sink.get(), -1.234 - 5.678);
}


BOOST_AUTO_TEST_SUITE_END()
