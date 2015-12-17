// boost
#include <boost/test/unit_test.hpp>

// fc
#include <core/connection.hpp>
#include <ports/states/state_sources.hpp>

using namespace fc;

namespace
{
/**
 * Simple node sending two for testing purposes.
 *
 * Serves also as example for state_source_tmpl port.
 */
class gives_two
{
public:
	gives_two() {}

	/*
	 * Converts two (2) to the type given and returns it as state.
	 */
	auto out()
	{
		return pure::make_state_source_tmpl( [this](auto f) -> auto
		{
			return typename decltype(f)::type(2);
		} );
	}
};
} // unnamed namespace

BOOST_AUTO_TEST_SUITE(test_tokens_testing)

BOOST_AUTO_TEST_CASE( test_it )
{
	gives_two two;
	/*
	 * Use the Type proxy to define the requested type.
	 */
	BOOST_CHECK_EQUAL(two.out()( Type<int>{} ), 2);
	BOOST_CHECK_EQUAL(two.out()( Type<double>{} ), 2.);
	BOOST_CHECK((two.out()( Type<std::vector<int>>{} ) == std::vector<int>{0, 0}));
}

BOOST_AUTO_TEST_SUITE_END()

