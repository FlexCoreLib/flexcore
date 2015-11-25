#include <boost/test/unit_test.hpp>

//#include <3rdparty/range-v3/include/range/v3/all.hpp>

#include <core/connection.hpp>
#include <range/algorithm.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_rage)

BOOST_AUTO_TEST_CASE(test_algorithm)
{
	std::vector<int> vec = {-4, -3, -2, -1, 0, 1, 2, 3, 4};

	auto source = [&vec](){ return boost::make_iterator_range(std::begin(vec), std::end(vec)); };
	auto sink = [](auto in){ for(int i : in) { std::cout << i << ", ";}};

	auto connection = source
				>> filter([](int i){ return i < 0;})
				>> map([](int i) { return i*2;})
				>> sink;
	connection();

	auto connection_2 = source
			>> filter([](int i){ return i < 0;})
			>> map([](int i){ return i*2;})
			>> sum(0)
			>> [](int i){std::cout << "sum: " << i << "\n"; };
	connection_2();

}

BOOST_AUTO_TEST_SUITE_END()
