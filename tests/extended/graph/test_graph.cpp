#include <boost/test/unit_test.hpp>
#include <boost/graph/graph_utility.hpp>

#include <flexcore/extended/graph/graph.hpp>
#include <flexcore/extended/graph/graph_connectable.hpp>
#include <flexcore/extended/base_node.hpp>
#include <flexcore/extended/nodes/terminal.hpp>
#include <flexcore/ports.hpp>
#include <flexcore/scheduler/cyclecontrol.hpp>

#include <boost/mpl/list.hpp>

namespace
{
	struct graph_fixture
	{
		fc::graph::connection_graph graph;
		fc::forest_owner forest{graph, "forest", std::make_shared<fc::parallel_region>("r",
				fc::thread::cycle_control::fast_tick)};
		std::ostringstream out_stream{};
	};
}

BOOST_FIXTURE_TEST_SUITE( test_graph, graph_fixture)

using fc::operator>>;

BOOST_AUTO_TEST_CASE(test_minimal_graph)
{
	auto factory = [&r = forest.nodes()](auto name) -> fc::state_terminal<int>&{
			return r.make_child_named<fc::state_terminal<int>>(name); };
	auto& source = factory("state_source 1");
	auto& sink = factory("state_source 2");

	source.out() >> sink.in();

	graph.print(out_stream);

	const auto dot_string = out_stream.str();
	const auto line_count =
			std::count(dot_string.begin(), dot_string.end(),'\n');

	// nr of lines in dot graph is nr of nodes and named lambdas
	// + nr of connections + 2 (one for begin one for end)
	BOOST_CHECK_EQUAL(line_count, 2 + 1 + 2);
}

using nodes = boost::mpl::list<fc::state_terminal<int>, fc::event_terminal<int>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_graph_creation, node_t, nodes)
{
	auto factory = [&r = forest.nodes()](auto name) -> node_t&{
			return r.template make_child_named<node_t>(name); };
	auto& source_1 = factory("state_source 1");
	auto& source_2 = factory("state_source 2");
	auto& intermediate = factory("intermediate");
	auto& sink = factory("state_sink");

	source_1.out() >> [](int i){ return i; } >> intermediate.in();
	source_2.out() >> (fc::graph::named([](int i){ return i; }, "incr") >> intermediate.in());
	intermediate.out() >>
			(fc::graph::named([](int i){ return i; }, "l 1") >>
					fc::graph::named([](int i){ return i; }, "l 2")) >> sink.in();

	using graph_source =  fc::graph::graph_connectable<fc::pure::event_source<int>>;
	using graph_sink = fc::graph::graph_connectable<fc::pure::event_sink<int>>;

	auto g_source = graph_source{graph, fc::graph::graph_node_properties{"event_source"}};
	int test_val{0};
	auto g_sink = graph_sink{graph, fc::graph::graph_node_properties{"event_sink"},
			[&test_val](int i){test_val = i;}};

	g_source >> fc::graph::named([](int i){ return i; }, "l 3") >> g_sink;

	graph.print(out_stream);
	g_source.fire(1);

	const auto dot_string = out_stream.str();
	const auto line_count =
			std::count(dot_string.begin(), dot_string.end(),'\n');

	BOOST_CHECK_EQUAL(test_val, 1);

	// nr of lines in dot graph is nr of nodes and named lambdas
	// + nr of connections + 2 (one for begin one for end)
	BOOST_CHECK_EQUAL(line_count, 10 + 8 + 2);
}

BOOST_AUTO_TEST_SUITE_END()
