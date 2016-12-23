#include <boost/test/unit_test.hpp>

#include <flexcore/core/connection.hpp>
#include <flexcore/range/actions.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_range)

BOOST_AUTO_TEST_CASE(test_actions)
{
	std::vector<int> vec {-4, -3, -2, -1, 0, 1, 2, 3, 4};

	auto con = actions::filter([](int i){ return i < 0;})
			>> actions::map([](int i){ return i*2;})
			>> sum(0);

	BOOST_CHECK_EQUAL(con(vec), -20);


	auto type_change = actions::map(
			[](int i){ return static_cast<float>(i);}, std::vector<float>{});

	std::vector<float> result = {-4, -3, -2, -1, 0, 1, 2, 3, 4};
	BOOST_CHECK(type_change(vec) == result);
}

BOOST_AUTO_TEST_CASE(test_zip)
{
	std::vector<int> vec {0, 1, 2, 3, 4};
	std::vector<int> squared_vec {0,1,4,9,16};

	auto source = [vec](){ return vec; };

	auto connection = source
			>> actions::zip([](auto a, auto b){return a*b;}, vec);

	std::vector<int> result = connection();
	BOOST_CHECK(result == squared_vec);
}


BOOST_AUTO_TEST_SUITE_END()
