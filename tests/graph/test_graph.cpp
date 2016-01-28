#include <boost/test/unit_test.hpp>

#include <graph/graph.hpp>
#include <graph/graph_connectable.hpp>

#include <boost/graph/graph_utility.hpp>

#include <nodes/base_node.hpp>
#include <ports/ports.hpp>


using namespace fc;

BOOST_AUTO_TEST_SUITE( test_graph )

BOOST_AUTO_TEST_CASE(test_graph_connectable)
{
//	auto source = graph::named([](){return 1;}, "give 1");
//	auto sink = graph::named([](int i){ std::cout << i;},"cout");
//
//	auto con = source >> [](int i){ return i+1; } >> sink;
//	con();
//
//	graph::print();
}

namespace
{
	struct dummy_node : public tree_base_node
	{
		dummy_node(const std::string& name)
			: tree_base_node(name)
			, out_port(this, 0)
			, in_port(this)
		{
		}

		auto& out() { return out_port; }
		auto& in() { return in_port; }

		state_source_with_setter<int> out_port;
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

	graph::print();

}

BOOST_AUTO_TEST_SUITE_END()
