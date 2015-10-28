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

BOOST_AUTO_TEST_SUITE_END()
