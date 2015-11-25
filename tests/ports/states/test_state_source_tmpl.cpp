// boost
#include <boost/test/unit_test.hpp>

// fc
#include <core/connection.hpp>
#include <ports/state_ports.hpp>

using namespace fc;

namespace
{
/**
 * Simple node sending two for testing purposes.
 */
class gives_two
{
public:
	gives_two()
		: out(*this)
	{}

	state_source_tmpl<gives_two> out;

	template<class tag_t, class token_t>
	token_t detail_out()
	{
		return token_t(2);
	}
};
} // unnamed namespace

BOOST_AUTO_TEST_SUITE(test_tokens_testing)

BOOST_AUTO_TEST_CASE( test_it )
{
	gives_two two;

	BOOST_CHECK_EQUAL(two.out.template operator()<int>(), 2);
	BOOST_CHECK_EQUAL(two.out.template operator()<double>(), 2.);
	BOOST_CHECK((two.out.template operator()<std::vector<int>>() == std::vector<int>{0, 0}));
}

BOOST_AUTO_TEST_SUITE_END()

