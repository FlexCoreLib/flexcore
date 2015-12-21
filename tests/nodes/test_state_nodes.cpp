#include <boost/test/unit_test.hpp>

#include <nodes/state_nodes.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_state_nodes )

BOOST_AUTO_TEST_CASE( test_merge )
{
	root_node root;
	auto multiply = root.add_child(merge( [](int a, int b){return a*b;} ));
	state_source_with_setter<int> three(&root, 3);
	state_source_with_setter<int> two(&root, 2);
	three >> multiply->in<0>();
	two >> multiply->in<1>();
	BOOST_CHECK_EQUAL((*multiply)(), 6);
}

BOOST_AUTO_TEST_SUITE_END()
