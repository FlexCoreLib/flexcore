#include <boost/test/unit_test.hpp>

#include <flexcore/core/connection.hpp>
#include <flexcore/pure/pure_ports.hpp>

#include <algorithm>
#include <vector>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_tokens_testing)

namespace
{
/**
 * token for testing that cannot be copied only moved.
 */
class move_token
{
public:
	explicit move_token(std::string v) noexcept : value_(std::move(v)) {}
	move_token() = default;
	move_token(move_token&) = delete;
	move_token(move_token&&) = default;
	move_token& operator= (move_token&) = delete;
	move_token& operator= (move_token&&) = default;

	std::string& value() { return value_; }
	const std::string& value() const { return value_; }

	bool operator< (const move_token& other) const { return value_ < other.value_; }

private:
	std::string value_;
};
}
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
	auto set_bar = [](auto&& t) { t.value() = "bar"; return std::move(t);};

	pure::event_source<move_token&&> source;

	move_token result{};

	pure::event_sink<move_token> sink{[&result](move_token&& in){ result = std::move(in);}};

	source >> set_bar >> sink;
	move_token m{"foo"};
	// source.fire(m); // should not compile
	source.fire(std::move(m));

	BOOST_CHECK_EQUAL(result.value(), "bar");
}

BOOST_AUTO_TEST_CASE( moving_state )
{
	auto set_bar = [](auto&& t) { t.value() = "bar"; return std::move(t); };
	pure::state_sink<move_token> sink;
	pure::state_source<move_token> source([](){ return move_token("foo"); });

	source >> set_bar >> sink;
	const auto v = sink.get();
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
