#include <boost/test/unit_test.hpp>

#include <nodes/state_nodes.hpp>


using namespace fc;

BOOST_AUTO_TEST_SUITE(test_state_nodes)

BOOST_AUTO_TEST_CASE(test_merge)
{
	auto multiply = merge([](int a, int b){return a*b;});
	[](){ return 3;} >> multiply.in<0>();
	[](){ return 2;} >> multiply.in<1>();
	BOOST_CHECK_EQUAL(multiply(), 6);
}

BOOST_AUTO_TEST_CASE(test_state_cache)
{
	state_cache<int> cache;

	int test_val = 1;

	[&test_val](){ return test_val;} >> cache.in();
	BOOST_CHECK_EQUAL(cache.out()(), 1);

	test_val = 0;
	BOOST_CHECK_EQUAL(cache.out()(), 1); //cache has not been reset

	cache.switch_tick()(); //reset cache
	BOOST_CHECK_EQUAL(cache.out()(), 0);
}

BOOST_AUTO_TEST_CASE(test_current_state)
{
	{ //check constructor
	current_state<int> test_node_1{1};
	BOOST_CHECK_EQUAL(test_node_1.out()(), 1);

	current_state<int> test_node_2;
	BOOST_CHECK_EQUAL(test_node_2.out()(), 0); //default is int() == 0
	}

	current_state<int> test_node;
	BOOST_CHECK_EQUAL(test_node.out()(), 0);

	int test_val = 1;
	auto source = [&test_val](){ return test_val;};
	source >> test_node.in();

	// we haven't pulled yet
	BOOST_CHECK_EQUAL(test_node.out()(), 0);

	test_node.pull()();
	BOOST_CHECK_EQUAL(test_node.out()(), 1);

	test_val = 2;
	BOOST_CHECK_EQUAL(test_node.out()(), 1);
	test_node.pull()();
	BOOST_CHECK_EQUAL(test_node.out()(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
