#include <boost/test/unit_test.hpp>

#include <graph/graph.hpp>
#include <graph/graph_connectable.hpp>

#include <boost/graph/graph_utility.hpp>

#include <nodes/base_node.hpp>
#include <ports/ports.hpp>


using namespace fc;

BOOST_AUTO_TEST_SUITE( test_graph )

namespace
{
	struct dummy_node : tree_base_node
	{
		dummy_node(const std::string& name)
			: tree_base_node(name)
			, out_port(this, [](){ return 0;})
			, in_port(this)
		{
		}

		auto& out() { return out_port; }
		auto& in() { return in_port; }

		state_source<int> out_port;
		state_sink<int> in_port;
		dummy_node(const dummy_node&) = delete;
	};

}

BOOST_AUTO_TEST_CASE(test_graph_creation)
{
	dummy_node source_1{"source 1"};
	dummy_node source_2{"source 2"};
	dummy_node intermediate{"intermediate"};
	dummy_node sink{"sink"};

	source_1.out() >> [](int i){ return i; } >> intermediate.in();
	source_2.out() >> (graph::named([](int i){ return i; }, "incr") >> intermediate.in());
	intermediate.out() >>
			(graph::named([](int i){ return i; }, "foo") >>
			graph::named([](int i){ return i; }, "bar")) >> sink.in();


	typedef graph::graph_connectable<pure::event_source<int>> graph_source;
	typedef graph::graph_connectable<pure::event_sink<int>> graph_sink;

	auto g_source = graph_source{graph::graph_node_properties{"foo"}};
	auto g_sink = graph_sink{graph::graph_node_properties{"bar"}, [](int i){}};

	g_source >> graph::named([](int i){ return i; }, "moep") >> g_sink;

	graph::print();
	g_source.fire(1);

}

BOOST_AUTO_TEST_SUITE_END()
