#include <boost/test/unit_test.hpp>

#include <flexcore/pure/state_sink.hpp>
#include <flexcore/pure/state_sources.hpp>
#include <flexcore/core/connection.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_state_sinks )

BOOST_AUTO_TEST_CASE( test_port_trait )
{
	typedef pure::state_sink<int> port_t;

	static_assert(  is_active_sink<port_t>{}, "");
	static_assert(! is_active_source<port_t>{}, "");
	static_assert(! is_passive_sink<port_t>{}, "");
	static_assert(! is_passive_source<port_t>{}, "");
}

BOOST_AUTO_TEST_CASE( test_disconnecting_state_ports )
{
	using sink_t = pure::state_sink<int>;
	using source_t = pure::state_source<int>;
	sink_t sink1, sink2;
	{
		source_t source{[]() { return 1; }};
		source >> sink1;
		source >> sink2;
	}

	BOOST_CHECK_THROW(sink1.get(), std::bad_function_call);
	BOOST_CHECK_THROW(sink2.get(), std::bad_function_call);
}

BOOST_AUTO_TEST_CASE( polymorphic_connectables)
{
	pure::state_source<double> source{[](){ return 2.;}};
	pure::state_sink<double> sink;

	source >> [](auto in){ return in*in; } >> sink;

	BOOST_CHECK_EQUAL(sink.get(), 4.);
}

BOOST_AUTO_TEST_CASE(connect_after_move)
{
	pure::state_sink<int> s1;
	pure::state_sink<int> s2 = std::move(s1);
	pure::state_source<int> src([] { return 99; });
	src >> s2;
	BOOST_CHECK_EQUAL(s2.get(), 99);
}

BOOST_AUTO_TEST_CASE(move_active_after_connect)
{
	pure::state_sink<int> s1;
	pure::state_source<int> src([] { return 99; });
	src >> s1;
	pure::state_sink<int> s2 = std::move(s1);
	BOOST_CHECK_EQUAL(s2.get(), 99);
	BOOST_CHECK_THROW(s1.get(), std::bad_function_call);
}

BOOST_AUTO_TEST_SUITE_END()
