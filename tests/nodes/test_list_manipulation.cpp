#include <boost/test/unit_test.hpp>

#include <nodes/list_manipulation.hpp>
#include <nodes/buffer.hpp>

// std
#include <deque>
#include <list>
#include <forward_list>

using namespace fc;

namespace
{

/*
 * Helper for comparing thwo ranges
 */
template<class left_t, class right_t>
void range_compare(const left_t& left, const right_t& right)
{
	typedef decltype(std::begin(left)) left_iter_t;
	typedef decltype(std::begin(right)) right_iter_t;
	left_iter_t left_it = std::begin(left);
	right_iter_t right_it = std::begin(right);

	while	(	left_it != std::end(left)
			and	right_it != std::end(right)
			)
	{
		BOOST_CHECK_EQUAL(*left_it, *right_it);
		++left_it;
		++right_it;
	}
	BOOST_CHECK(left_it == std::end(left));
	BOOST_CHECK(right_it == std::end(right));
}

} // unnamed namespace

BOOST_AUTO_TEST_SUITE( test_list_manipulation )

BOOST_AUTO_TEST_CASE( test_list_splitter )
{
	typedef list_splitter <std::list<std::string>, size_t> splitter_t;
	auto predicate = [](const std::string& s) { return s.size(); };
	splitter_t splitter(predicate);

	std::vector<std::vector<std::string>> output(5);
	typedef decltype(splitter.out(0))::result_t out_range_t;

	for (size_t i = 0; i < 5; ++i)
		splitter.out(i) >> [&output, i](const out_range_t& v) // [&output, i](const auto& v)
				{ output.at(i) = std::vector<std::string>(boost::begin(v), boost::end(v)); };

	// send data
	std::list<std::string> input { "aa", "bbb", "c", "d", "too long 1", "too long 2  " };
	splitter.in(input);

	// check result
	range_compare(output.at(0), std::list<std::string>{ });
	range_compare(output.at(1), std::list<std::string>{"c", "d"});
	range_compare(output.at(2), std::list<std::string>{"aa"});
	range_compare(output.at(3), std::list<std::string>{"bbb"});
	range_compare(output.at(4), std::list<std::string>{ });

	BOOST_CHECK_EQUAL(splitter.out_num_dropped(), 2);

	// send again
	std::list<std::string> input2 { "a", "b", "cd", "too long 3" };
	splitter.in(input2);

	// check result
	range_compare(output.at(0), std::list<std::string>{ });
	range_compare(output.at(1), std::list<std::string>{"a", "b"});
	range_compare(output.at(2), std::list<std::string>{"cd"});

	BOOST_CHECK_EQUAL(splitter.out_num_dropped(), 3);
}

/*
 * test for predicate_result == bool
 */
BOOST_AUTO_TEST_CASE( test_list_splitter_bool )
{
	typedef list_splitter <std::list<int>, bool> splitter_t;
	auto predicate = [](int v) { return v >= 0; };
	splitter_t splitter(predicate);

	std::vector<int> out_true;
	std::vector<int> out_false;
	typedef decltype(splitter.out(0))::result_t out_range_t;

	splitter.out(true) >> [&out_true](const out_range_t& v)
			{ out_true = std::vector<int>(boost::begin(v), boost::end(v)); };
	splitter.out(false) >> [&out_false](const out_range_t& v)
			{ out_false = std::vector<int>(boost::begin(v), boost::end(v)); };

	// send data
	std::list<int> input { 1, -1, 5, -6, -6, 1 };
	splitter.in(input);

	range_compare(out_true, std::vector<int>{ 1, 5, 1 });
	range_compare(out_false, std::vector<int>{ -1, -6, -6});
}

BOOST_AUTO_TEST_CASE( test_list_collector )
{
	typedef list_splitter <std::list<int>, bool> splitter_t;
	auto predicate = [](int) { return 0; }; // always return 0
	splitter_t splitter(predicate);

	typedef list_collector<int, swap_on_pull> collector_t;
	collector_t collector;

	state_sink<collector_t::out_range_t> sink;

	splitter.out(0) >> collector.in();
	collector.out() >> sink;

	// send data
	splitter.in( std::list<int>{1, 2} );
	splitter.in( std::list<int>{3} );

	range_compare(sink.get(), std::forward_list<int>{ 1, 2, 3 });

	splitter.in( std::list<int>{4, 5} );

	range_compare(sink.get(), std::forward_list<int>{ 4, 5 });
	range_compare(sink.get(), std::forward_list<int>{ });
}

BOOST_AUTO_TEST_SUITE_END()

