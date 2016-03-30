#include <boost/test/unit_test.hpp>

#include <nodes/pure_node.hpp>
#include <nodes/list_manipulation.hpp>
#include <nodes/buffer.hpp>

#include "owning_node.hpp"

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
	auto left_it = std::begin(left);
	auto right_it = std::begin(right);

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
	typedef list_splitter <std::list<std::string>, size_t, pure::pure_node> splitter_t;
	auto predicate = [](const std::string& s) { return s.size(); };
	auto splitter = splitter_t{predicate};

	std::vector<std::vector<std::string>> output(5);

	for (size_t i = 0; i < 5; ++i)
		splitter.out(i) >> [&output, i](const typename splitter_t::out_range_t& v)
				{ output.at(i) = std::vector<std::string>(std::begin(v), std::end(v)); };

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
	typedef list_splitter <std::list<int>, bool, pure::pure_node> splitter_t;
	auto predicate = [](int v) { return v >= 0; };
	auto splitter = splitter_t{predicate};

	std::vector<int> out_true;
	std::vector<int> out_false;

	splitter.out(true) >> [&out_true](const typename splitter_t::out_range_t& v)
			{ out_true = std::vector<int>(boost::begin(v), boost::end(v)); };
	splitter.out(false) >> [&out_false](const typename splitter_t::out_range_t& v)
			{ out_false = std::vector<int>(boost::begin(v), boost::end(v)); };

	// send data
	std::list<int> input { 1, -1, 5, -6, -6, 1 };
	splitter.in(input);

	range_compare(out_true, std::vector<int>{ 1, 5, 1 });
	range_compare(out_false, std::vector<int>{ -1, -6, -6});
}

BOOST_AUTO_TEST_CASE( test_list_collector_pure )
{
	// test case of list collector without region context

	typedef list_splitter <std::list<int>, bool, pure::pure_node> splitter_t;
	typedef list_collector<int, swap_on_pull, pure::pure_node> collector_t;

	splitter_t splitter{ [](int) { return 0; }};
	collector_t collector;

	pure::state_sink<collector_t::out_range_t> sink;

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

