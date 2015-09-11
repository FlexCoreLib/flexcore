// boost
#include <boost/test/unit_test.hpp>

#include <ports/dev.hpp>
#include <core/connection.hpp>

BOOST_AUTO_TEST_CASE( fetcher_ )
{
	auto give_one = [](){return 1;};
	fetcher<int> sink;
	connect(give_one, sink);
	BOOST_CHECK(sink.get() == 1);
}

BOOST_AUTO_TEST_CASE( state_ )
{
	state<int> source {1};
	fetcher<int> sink;
	connect(source, sink);
	BOOST_CHECK(sink.get() == 1);

}



