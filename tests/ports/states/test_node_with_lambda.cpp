// std
#include <functional>

// boost
#include <boost/test/unit_test.hpp>

#include <core/connection.hpp>
#include <ports/states/state_sink.hpp>

using namespace fc;

namespace // unnamed
{
template<class data_t>
struct node_class
{
	node_class(data_t v = data_t())
		: value(v)
	{}

	data_t get_value() { return value; }

	auto port()
	{
		return [this](){ return this->get_value(); };
	}

	data_t value;
};
} // unnamed namespace

BOOST_AUTO_TEST_CASE( node_with_lambda_to_member )
{
	node_class<int> node;
	auto increment = [](int i) -> int { return i+1; };
	pure::state_sink<int> sink;

	static_assert(is_passive<decltype(node.port())>{},
			"returns a lambda which takes void param, which is passive");
	node.port() >> increment >> sink;

	BOOST_CHECK(sink.get() == 1);
	node.value = 5;
	BOOST_CHECK(sink.get() == 6);
}
