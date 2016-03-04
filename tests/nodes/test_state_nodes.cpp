#include <boost/test/unit_test.hpp>

#include <nodes/state_nodes.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_state_nodes )

BOOST_AUTO_TEST_CASE( test_merge )
{
	root_node root;
	auto multiply = make_merge( root, [](int a, int b){return a*b;} );
	state_source<size_t> three(&root, [](){ return 3; });
	state_source<size_t> two(&root, [](){ return 2; });
	three >> multiply->in<0>();
	two >> multiply->in<1>();
	BOOST_CHECK_EQUAL((*multiply)(), 6);
}

BOOST_AUTO_TEST_CASE(test_state_cache)
{
	root_node root;
	auto& cache = *(root.make_child<state_cache<int>>());

	int test_val = 1;

	[&test_val](){ return test_val;} >> cache.in();
	BOOST_CHECK_EQUAL(cache.out()(), 1);

	test_val = 0;
	BOOST_CHECK_EQUAL(cache.out()(), 1); //cache has not been reset

	cache.update()(); //reset cache
	BOOST_CHECK_EQUAL(cache.out()(), 0);
}

BOOST_AUTO_TEST_CASE(test_current_state)
{
	root_node root;
	{ //check constructor
	auto& test_node_1 = *(root.make_child<current_state<int>>(1));
	BOOST_CHECK_EQUAL(test_node_1.out()(), 1);

	auto& test_node_2 = *(root.make_child<current_state<int>>());
	BOOST_CHECK_EQUAL(test_node_2.out()(), int()); //default is int()
	}

	auto& test_node = *(root.make_child<current_state<int>>());
	BOOST_CHECK_EQUAL(test_node.out()(), 0);

	int test_val = 1;
	auto source = [&test_val](){ return test_val;};
	source >> test_node.in();

	// we haven't updated the cache yet
	BOOST_CHECK_EQUAL(test_node.out()(), int()); //default is int()

	test_node.update()();
	BOOST_CHECK_EQUAL(test_node.out()(), 1);

	test_val = 2;
	BOOST_CHECK_EQUAL(test_node.out()(), 1);
	test_node.update()();
	BOOST_CHECK_EQUAL(test_node.out()(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
