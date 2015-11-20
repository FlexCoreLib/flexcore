// boost
#include <boost/test/unit_test.hpp>

// fc
#include <core/connection.hpp>
#include <ports/state_ports.hpp>

using namespace fc;

BOOST_AUTO_TEST_CASE( fetcher_ )
{
	std::function<int()> give_one = [](){return 1;};
	state_sink<int> sink;
	give_one >> sink;
	BOOST_CHECK(sink.get() == 1);
}

BOOST_AUTO_TEST_CASE( state_fetcher_direct_connection )
{
	state_source_with_setter<int> source {1};
	state_sink<int> sink;
	source >> sink;
	BOOST_CHECK(sink.get() == 1);
	source.set(2);
	BOOST_CHECK(sink.get() == 2);
}

BOOST_AUTO_TEST_CASE( state_multiple_fetchers_and_assignment )
{
	auto increment = [](int i) -> int {return i+1;};
	state_source_with_setter<int> source {1};
	state_sink<int> sink1;
	state_sink<int> sink2;
	source >> increment >> sink1;
	source >> increment >> increment >> sink2;
	BOOST_CHECK(sink1.get() == 2);
	BOOST_CHECK(sink2.get() == 3);
	source.set(5);
	BOOST_CHECK(sink1.get() == 6);
	BOOST_CHECK(sink2.get() == 7);
}

BOOST_AUTO_TEST_CASE( state_fetcher_stored_sink_connection )
{
	state_source_with_setter<int> source {1};
	auto increment = [](int i) -> int {return i+1;};
	state_sink<int> sink;

	auto tmp = (increment >> sink);
	static_assert(is_instantiation_of<
			detail::active_connection_proxy, decltype(tmp)>::value,
			"active sink connected with standard connectable gets proxy");
	source >> tmp;

	BOOST_CHECK(sink.get() == 2);
	source.set(2);
	BOOST_CHECK(sink.get() == 3);

	state_source_with_setter<int> source_2 {1};
	auto tmp_connetion = (increment >> increment);

	static_assert(!void_callable<decltype(increment)>(0), "");
	static_assert(!void_callable<decltype(1)>(0), "");
	//static_assert(void_callable<decltype(tmp_connetion)>(0), "");

//	auto two_stored = tmp_connetion >> sink;
//	source_2 >> two_stored;
//	BOOST_CHECK_EQUAL(sink.get(), 3);
//
//	state_source_with_setter<int> source_3 {2};
//	auto tmp2 = (increment >> sink);
//	auto two_stored_2 = increment >> tmp2;
//	source_3 >> two_stored_2;
//	BOOST_CHECK_EQUAL(sink.get(), 4);

}
