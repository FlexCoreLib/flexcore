#include <boost/test/unit_test.hpp>

#include <flexcore/pure/state_sink.hpp>
#include <flexcore/pure/state_sources.hpp>
#include <flexcore/core/connection.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_state_sinks )

BOOST_AUTO_TEST_CASE( test_port_trait )
{
	using port_t = pure::state_sink<int>;

	static_assert( is_active_sink<port_t>{}, "");
	static_assert(!is_active_source<port_t>{}, "");
	static_assert(!is_passive_sink<port_t>{}, "");
	static_assert(!is_passive_source<port_t>{}, "");
}

BOOST_AUTO_TEST_CASE( test_disconnecting_state_ports )
{
	using sink_t = pure::state_sink<int>;
	using source_t = pure::state_source<int>;
	sink_t sink1{};
	sink_t sink2{};
	{
		source_t source{[]() { return 1; }};
		source >> sink1;
		source >> sink2;
	}

	BOOST_CHECK_THROW(sink1.get(), fc::not_connected);
	BOOST_CHECK_THROW(sink2.get(), fc::not_connected);
}

BOOST_AUTO_TEST_CASE( polymorphic_connectables)
{
	pure::state_source<double> source{[](){ return 2.;}};
	pure::state_sink<double> sink{};

	source >> [](auto in){ return in*in; } >> sink;

	BOOST_CHECK_EQUAL(sink.get(), 4.);
}

BOOST_AUTO_TEST_CASE(connect_after_move)
{
	pure::state_sink<int> s1{};
	pure::state_sink<int> s2 = std::move(s1);
	pure::state_source<int> src([] { return 99; });
	src >> s2;
	BOOST_CHECK_EQUAL(s2.get(), 99);
}

BOOST_AUTO_TEST_CASE(moving_state_source)
{
	pure::state_sink<int> sink{};
	pure::state_source<int> src([] { return 99; });
	pure::state_source<int> src_2([] { return 1; });
	src >> sink;
	BOOST_CHECK_EQUAL(sink.get(), 99);

	src = std::move(src_2);
	BOOST_CHECK_EQUAL(sink.get(), 1);
}


BOOST_AUTO_TEST_CASE(move_active_after_connect)
{
	pure::state_sink<int> s1{};
	pure::state_source<int> src([] { return 99; });
	src >> s1;
	pure::state_sink<int> s2 = std::move(s1);
	BOOST_CHECK_EQUAL(s2.get(), 99);
	BOOST_CHECK_THROW(s1.get(), fc::not_connected);
}

BOOST_AUTO_TEST_CASE( stream_query_multiple_sinks )
{
	int test_value{0};
	pure::state_source<int> source{[&test_value](){return test_value; }};
	auto increment = [](int i) -> int { return i+1; };
	pure::state_sink<int> sink1{};
	pure::state_sink<int> sink2{};

	source >> increment >> sink1;
	source >> increment >> sink2;

	BOOST_CHECK_EQUAL(sink1.get(), test_value+1);
	BOOST_CHECK_EQUAL(sink2.get(), test_value+1);
	test_value = 5;
	BOOST_CHECK_EQUAL(sink1.get(), test_value+1);
	BOOST_CHECK_EQUAL(sink2.get(), test_value+1);
}

BOOST_AUTO_TEST_CASE( state_sink_direct_connection )
{
	int state{1};
	pure::state_source<int> source {[&state](){ return state;}};
	pure::state_sink<int> sink{};
	source >> sink;
	BOOST_CHECK(sink.get() == state);
	state = 2;
	BOOST_CHECK(sink.get() == state);
}

BOOST_AUTO_TEST_CASE( state_sink_stored_sink_connection )
{
	int state{1};
	pure::state_source<int> source {[&state](){ return state;}};
	auto increment = [](int i) -> int {return i+1;};
	pure::state_sink<int> sink{};

	auto tmp = (increment >> sink);
	static_assert(is_instantiation_of<
			detail::active_connection_proxy, decltype(tmp)>{},
			"active sink connected with standard connectable gets proxy");
	source >> std::move(tmp);

	BOOST_CHECK(sink.get() == 2);
	state = 2;
	BOOST_CHECK(sink.get() == state+1);

	pure::state_source<int> source_2 {[&state](){ return 1;}};
	auto connection_add2 = (increment >> increment);

	static_assert(!void_callable<decltype(increment)>(0), "");
	static_assert(!void_callable<decltype(1)>(0), "");
	static_assert(!void_callable<decltype(connection_add2)>(0), "");

	source_2 >> (connection_add2 >> sink);
	BOOST_CHECK_EQUAL(sink.get(), 3);

	pure::state_source<int> source_3 {[&state](){ return state;}};
	source_3 >> (increment >> (increment >> sink));
	BOOST_CHECK_EQUAL(sink.get(), 4);
}

BOOST_AUTO_TEST_CASE(state_source_connect_after_move)
{
	pure::state_source<int> src{[] { return 99; }};
	pure::state_sink<int> sink{};
	auto new_src = std::move(src);
	new_src >> sink;
	BOOST_CHECK_EQUAL(sink.get(), 99);
	// object after move is in valid state, but should not have access to the
	// old callback anymore.
	src >> sink;
	BOOST_CHECK_THROW(sink.get(), std::bad_function_call);
}

BOOST_AUTO_TEST_SUITE_END()
