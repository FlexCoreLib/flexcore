#include <boost/test/unit_test.hpp>

#include <ports/states/state_sink.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_state_sinks )

BOOST_AUTO_TEST_CASE( test_state_sink )
{
	typedef pure::state_sink<int> port_t;

	static_assert(is_active_sink<port_t>{}, "");
	static_assert(not is_active_source<port_t>{}, "");
	static_assert(not is_passive_sink<port_t>{}, "");
	static_assert(not is_passive_source<port_t>{}, "");
}

BOOST_AUTO_TEST_SUITE_END()
