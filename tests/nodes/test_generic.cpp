#include <boost/test/unit_test.hpp>

#include <nodes/generic.hpp>


using namespace fc;

BOOST_AUTO_TEST_SUITE(test_generic_nodes)

BOOST_AUTO_TEST_CASE(test_transform)
{
	auto multiply = transform([](int a, int b){ return a * b;});

	[](){ return 3; } >> multiply.param;

	BOOST_CHECK_EQUAL(multiply(2), 6);

	auto add = transform([](int a, int b){ return a + b;});

	auto con = [](){return 4;} >> add >> [](int i) { return i+1; };
	[](){ return 3; } >> add.param;

	BOOST_CHECK_EQUAL(con(), 8);
}

BOOST_AUTO_TEST_SUITE_END()
