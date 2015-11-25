#include <boost/test/unit_test.hpp>

//#include <3rdparty/range-v3/include/range/v3/all.hpp>

#include <core/connection.hpp>
#include <ports/state_ports.hpp>

#include <range/algorithm.hpp>
#include <boost/range/any_range.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_rage)

BOOST_AUTO_TEST_CASE(test_algorithm)
{
	std::vector<int> vec {-4, -3, -2, -1, 0, 1, 2, 3, 4};

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
			>> sum(0);
	BOOST_CHECK_EQUAL(connection_2(), -20);
}

BOOST_AUTO_TEST_CASE(test_ports_with_ranges)
{
	std::vector<int> vec {-4, -3, -2, -1, 0, 1, 2, 3, 4};

	typedef boost::any_range<
			int,
			boost::single_pass_traversal_tag,
			int,
			std::ptrdiff_t
			> any_int_range;

	state_source_call_function<any_int_range>
		source([&vec]()
		{
			return boost::make_iterator_range(std::begin(vec), std::end(vec));
		});

	state_sink<any_int_range> sink;

	source
		>> filter([](int i){ return i < 0;})
		>> map([](int i) { return i*2;})
		>> sink;

	std::cout << sink.get() << "\n";
}

BOOST_AUTO_TEST_SUITE_END()
