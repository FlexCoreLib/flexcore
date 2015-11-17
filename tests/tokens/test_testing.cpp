#include <boost/test/unit_test.hpp>

#include <tokens/testing.hpp>

// std
#include <vector>
#include <algorithm>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_tokens_testing)

BOOST_AUTO_TEST_CASE(test_move_token)
{
	// no default constructor
//	move_token t;

	// non-copyable
	move_token t("foo");
	move_token u(move_token("bar"));

	std::vector<move_token> v(10);
	std::sort(v.begin(), v.end());
	v.push_back(move_token("foo"));
}

BOOST_AUTO_TEST_SUITE_END()







