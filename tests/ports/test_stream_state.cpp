// boost
#include <boost/test/unit_test.hpp>

#include <ports/stream_ports.hpp>
#include <core/connection.hpp>

using namespace fc;

BOOST_AUTO_TEST_CASE( fetcher_ )
{
	auto give_one = [](){return 1;};
	stream_sink<int> sink;
	give_one >> sink;
	BOOST_CHECK(sink.get() == 1);
}

BOOST_AUTO_TEST_CASE( state_fetcher_direct_connection )
{
	stream_state<int> source {1};
	stream_sink<int> sink;
	source >> sink;
	BOOST_CHECK(sink.get() == 1);
	source.set(2);
	BOOST_CHECK(sink.get() == 2);
}

BOOST_AUTO_TEST_CASE( state_multiple_fetchers_and_assignment )
{
	auto increment = [](int i) -> int {return i+1;};
	stream_state<int> source {1};
	stream_sink<int> sink1;
	stream_sink<int> sink2;
	source >> increment >> increment >> sink1;
	source >> increment >> increment >> sink2;
	BOOST_CHECK(sink1.get() == 3);
	BOOST_CHECK(sink2.get() == 3);
	source.set(5);
	BOOST_CHECK(sink1.get() == 7);
	BOOST_CHECK(sink2.get() == 7);
}





