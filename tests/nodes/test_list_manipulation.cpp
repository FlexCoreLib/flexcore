#include <boost/test/unit_test.hpp>

#include <flexcore/pure/pure_node.hpp>
#include <flexcore/extended/nodes/buffer.hpp>

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


BOOST_AUTO_TEST_CASE( test_list_collector_pure )
{
	// test case of list collector without region context

	typedef list_collector<int, swap_on_pull, pure::pure_node> collector_t;

	collector_t collector;

	pure::state_sink<std::vector<int>> sink;

	collector.out() >> sink;

	// send data
	collector.in()( std::vector<int>{1, 2} );
	collector.in()( std::vector<int>{3} );

	range_compare(sink.get(), std::vector<int>{ 1, 2, 3 });

	collector.in()( std::vector<int>{4, 5} );

	range_compare(sink.get(), std::vector<int>{ 4, 5 });
	range_compare(sink.get(), std::vector<int>{ });
}

BOOST_AUTO_TEST_SUITE_END()
