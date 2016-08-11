#include <boost/test/unit_test.hpp>

#include <flexcore/core/connection.hpp>
#include <flexcore/pure/state_sources.hpp>
#include <flexcore/pure/state_sink.hpp>

#include <flexcore/range/actions.hpp>

#include <boost/range/any_range.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

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
	std::vector<int> correct_result {0,1,4,9,16};

	auto source = [&vec](){ return boost::make_iterator_range(std::begin(vec), std::end(vec)); };

	auto connection = source
			>> actions::zip([](auto a, auto b){return a*b;}, vec);

	std::vector<int> result;
	boost::push_back(result, connection());
	BOOST_CHECK(result == correct_result);
}


BOOST_AUTO_TEST_SUITE_END()
