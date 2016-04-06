// boost
#include <boost/test/unit_test.hpp>

// fc
#include <flexcore/core/connection.hpp>
#include <flexcore/pure/state_sink.hpp>
#include <flexcore/pure/state_sources.hpp>

using namespace fc;

BOOST_AUTO_TEST_CASE( fetcher_ )
{
	std::function<int()> give_one = [](){return 1;};
	pure::state_sink<int> sink;
	give_one >> sink;
	BOOST_CHECK(sink.get() == 1);
}

BOOST_AUTO_TEST_CASE( state_fetcher_direct_connection )
{
	int state = 1;
	pure::state_source<int> source {[&state](){ return state;}};
	pure::state_sink<int> sink;
	source >> sink;
	BOOST_CHECK(sink.get() == 1);
	state = 2;
	BOOST_CHECK(sink.get() == 2);
}

BOOST_AUTO_TEST_CASE( state_multiple_fetchers_and_assignment )
{
	auto increment = [](int i) -> int {return i+1;};
	int state = 1;
	pure::state_source<int> source {[&state](){ return state;}};
	pure::state_sink<int> sink1;
	pure::state_sink<int> sink2;
	source >> increment >> sink1;
	source >> increment >> increment >> sink2;
	BOOST_CHECK(sink1.get() == 2);
	BOOST_CHECK(sink2.get() == 3);
	state = 5;
	BOOST_CHECK(sink1.get() == 6);
	BOOST_CHECK(sink2.get() == 7);
}

BOOST_AUTO_TEST_CASE( state_fetcher_stored_sink_connection )
{
	int state = 1;
	pure::state_source<int> source {[&state](){ return state;}};
	auto increment = [](int i) -> int {return i+1;};
	pure::state_sink<int> sink;

	auto tmp = (increment >> sink);
	static_assert(is_instantiation_of<
			detail::active_connection_proxy, decltype(tmp)>{},
			"active sink connected with standard connectable gets proxy");
	source >> std::move(tmp);

	BOOST_CHECK(sink.get() == 2);
	state = 2;
	BOOST_CHECK(sink.get() == 3);

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
