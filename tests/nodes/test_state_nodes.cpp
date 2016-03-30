#include <boost/test/unit_test.hpp>

#include <nodes/state_nodes.hpp>

#include "owning_node.hpp"

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_state_nodes )

BOOST_AUTO_TEST_CASE( test_merge )
{
	auto multiply = make_merge([](int a, int b){return a*b;} );
	pure::state_source<size_t> three([](){ return 3; });
	pure::state_source<size_t> two([](){ return 2; });
	mux(three, two) >> multiply.mux();
	BOOST_CHECK_EQUAL(multiply(), 6);
}

BOOST_AUTO_TEST_CASE(test_state_cache)
{
	state_cache<int, pure::pure_node> cache;

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
	auto region = std::make_shared<parallel_region>("MyRegion");

	{ //check constructor
	auto region_2 = std::make_shared<parallel_region>("MyRegion");
	current_state<int> test_node_1{region_2,1};
	BOOST_CHECK_EQUAL(test_node_1.out()(), 1);

	current_state<int> test_node_2{region_2};
	BOOST_CHECK_EQUAL(test_node_2.out()(), int()); //default is int()
	}

	current_state<int> test_node{region};
	BOOST_CHECK_EQUAL(test_node.out()(), 0);

	int test_val = 1;
	auto source = [&test_val](){ return test_val;};
	source >> test_node.in();

	// we haven't updated the cache yet
	BOOST_CHECK_EQUAL(test_node.out()(), int());

	region->ticks.work.fire(); //triggers update of the cache
	BOOST_CHECK_EQUAL(test_node.out()(), 1);

	test_val = 2;
	BOOST_CHECK_EQUAL(test_node.out()(), 1);
	region->ticks.work.fire(); //triggers update of the cache
	BOOST_CHECK_EQUAL(test_node.out()(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
