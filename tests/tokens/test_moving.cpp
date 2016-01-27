#include <boost/test/unit_test.hpp>

#include <core/connection.hpp>
#include <ports/events/event_sink_with_queue.hpp>
#include <vector>
#include <algorithm>

#include <ports/pure_ports.hpp>
#include "move_token.hpp"

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_tokens_testing)

BOOST_AUTO_TEST_CASE( move_token_ )
{
	// no default constructor
	// how to test for "does not compile"?
//	move_token t;

	// non-copyable
	move_token t("foo");
	move_token u(move_token("bar"));

	std::vector<move_token> v(10);
	std::sort(v.begin(), v.end());
	v.push_back(move_token("foo"));
}

BOOST_AUTO_TEST_CASE( moving_events )
{
	auto set_bar = [](auto&& t) { t.value() = "bar"; return std::move(t); };

	pure::event_source<move_token&&> source;

	pure::event_sink_queue<move_token> sink;

	std::function<void(move_token&&)> bla = sink;
	source >> set_bar >> bla;
	source.fire(move_token("foo"));

	move_token tmp = sink.get();
	BOOST_CHECK_EQUAL(tmp.value(), "bar");
}

BOOST_AUTO_TEST_CASE( moving_state )
{
	auto set_bar = [](auto&& t) { t.value() = "bar"; return std::move(t); };
	pure::state_sink<move_token> sink;
	pure::state_source_call_function<move_token> source([](){ return move_token("foo"); });

	source >> set_bar >> sink;
	auto v = sink.get();
	BOOST_CHECK_EQUAL(v.value(), "bar");
}


//test case to make sure objects are not move if we don't want them to be
BOOST_AUTO_TEST_CASE( non_moving )
{
	typedef std::shared_ptr<int> non_move; //shared_ptr is nulled after move, so we can check
	pure::event_source<non_move> source;
	bool moved = false;
	pure::event_sink<non_move> sink([&moved](non_move t){ moved = !t.operator bool() ;});
	pure::event_sink<non_move> sink2([&moved](non_move t){ moved = !t.operator bool() ;});

	source >> sink;

	source.fire(std::make_shared<int>(1));
	BOOST_CHECK(!moved);

	source >> sink2; //now we have two connections
	source.fire(std::make_shared<int>(1));
	BOOST_CHECK(!moved);
}

BOOST_AUTO_TEST_SUITE_END()
