#include <boost/test/unit_test.hpp>

#include <graph/graph.hpp>
#include <graph/graph_connectable.hpp>

#include <boost/graph/graph_utility.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_graph )

BOOST_AUTO_TEST_CASE(test_graph_connectable)
{
	auto source = graph::named([](){return 1;}, "give 1");
	auto sink = graph::named([](int i){ std::cout << i;},"cout");

	auto con = source >> [](int i){ return i+1; } >> sink;
	con();

	graph::print();
}

BOOST_AUTO_TEST_CASE(test_graph_creation)
{

}

BOOST_AUTO_TEST_SUITE_END()
