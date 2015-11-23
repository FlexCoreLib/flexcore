#include <boost/test/unit_test.hpp>

#include "core/connection.hpp"
#include "ports/events/event_sink_with_queue.hpp"
#include "ports/event_ports.hpp"
#include "ports/state_ports.hpp"

#include <vector>
#include <algorithm>
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
	auto set_bar = [](auto&& t) -> move_token&& { t.value() = "bar"; return std::move(t); };
//	auto set_bar = [](move_token&& t) { t.value() = "bar"; return std::move(t); };

	event_out_port<move_token&&> source;
	event_in_port<move_token> sink([](move_token&& t){ std::cout << "move: " << t.value() << "\n";});

	std::function<void(move_token&&)> bla = sink;
	source >> set_bar >> bla;
	source.fire(move_token("foo"));
}

BOOST_AUTO_TEST_CASE( moving_state )
{
	auto set_bar = [](auto&& t) -> move_token&& { t.value() = "bar"; return std::move(t); };
	state_sink<move_token> sink;
	state_source_call_function<move_token> source([](){ return move_token("foo"); });

	source >> set_bar >> sink;
	BOOST_CHECK_EQUAL(sink.get().value(), "bar");

}


//test case to make sure objects are not move if we don't want them to be
BOOST_AUTO_TEST_CASE( non_moving )
{
	typedef std::shared_ptr<int> non_move; //shared_ptr is nulled after move, so we can check
	event_out_port<non_move> source;
	bool moved = false;
	event_in_port<non_move> sink([&moved](non_move&& t){ moved = !t.operator bool() ;});
	event_in_port<non_move> sink2([&moved](non_move&& t){ moved = !t.operator bool() ;});

	source >> sink;

	source.fire(std::make_shared<int>(1));
	BOOST_CHECK(!moved);

	source >> sink2; //now we have two connections
	source.fire(std::make_shared<int>(1));
	BOOST_CHECK(!moved);

}


BOOST_AUTO_TEST_SUITE_END()







