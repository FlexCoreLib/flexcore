#include <boost/test/unit_test.hpp>
#include <boost/graph/graph_utility.hpp>

#include <flexcore/extended/graph/graph.hpp>
#include <flexcore/extended/graph/graph_connectable.hpp>
#include <flexcore/extended/base_node.hpp>
#include <flexcore/ports.hpp>


using namespace fc;

BOOST_AUTO_TEST_SUITE( test_graph )

namespace
{
	struct dummy_node : tree_base_node
	{
		explicit dummy_node(const node_args& node)
			: tree_base_node(node)
			, out_port(this, [](){ return 0;})
			, in_port(this)
		{
		}

		auto& out() { return out_port; }
		auto& in() { return in_port; }

		state_source<int> out_port;
		state_sink<int> in_port;
		dummy_node(const dummy_node&) = delete;

		static_assert(is_active<state_sink<int>>::value,
				"state_sink is active independent of mixins");
		static_assert(is_passive<state_source<int>>::value,
				"state_source is passive independent of mixins");
	};

}

BOOST_AUTO_TEST_CASE(test_graph_creation)
{
	graph::connection_graph graph;
	forest_owner forest{graph, "forest", std::make_shared<parallel_region>("r")};
	auto& r = forest.nodes();
	auto dummy_node_factory = [&](auto name) -> dummy_node&{ return r.make_child_named<dummy_node>(name); };
	dummy_node& source_1 = dummy_node_factory("state_source 1");
	dummy_node& source_2 = dummy_node_factory("state_source 2");
	dummy_node& intermediate = dummy_node_factory("intermediate");
	dummy_node& sink = dummy_node_factory("state_sink");

	source_1.out() >> [](int i){ return i; } >> intermediate.in();
	source_2.out() >> (graph::named([](int i){ return i; }, "incr") >> intermediate.in());
	intermediate.out() >>
			(graph::named([](int i){ return i; }, "l 1") >>
			graph::named([](int i){ return i; }, "l 2")) >> sink.in();

	typedef graph::graph_connectable<pure::event_source<int>> graph_source;
	typedef graph::graph_connectable<pure::event_sink<int>> graph_sink;

	auto g_source = graph_source{graph, graph::graph_node_properties{"event_source"}};
	int test_val = 0;
	auto g_sink = graph_sink{graph, graph::graph_node_properties{"event_sink"},
			[&test_val](int i){test_val = i;}};

	g_source >> graph::named([](int i){ return i; }, "l 3") >> g_sink;

	std::ostringstream out_stream;

	graph.print(out_stream);
	g_source.fire(1);

	auto dot_string = out_stream.str();
	unsigned line_count =
			std::count(dot_string.begin(), dot_string.end(),'\n');

	BOOST_CHECK_EQUAL(test_val, 1);

	// nr of lines in dot graph is nr of nodes and named lambdas
	// + nr of connections + 2 (one for begin one for end)
	BOOST_CHECK_EQUAL(line_count, 10 + 8 + 2);
}

BOOST_AUTO_TEST_SUITE_END()
