#include <boost/test/unit_test.hpp>

#include <flexcore/pure/event_sinks.hpp>
#include <flexcore/pure/event_sources.hpp>
#include <flexcore/core/connection.hpp>

#include <tests/pure/sink_fixture.hpp>

BOOST_AUTO_TEST_SUITE(test_events)

using namespace fc;

//several event sources can be connected to one sink
BOOST_AUTO_TEST_CASE( merge_events )
{
	pure::event_source<int> test_event{};
	pure::event_source<int> test_event_2{};
	pure::sink_fixture<int> test_sink;

	test_event >> test_sink;
	test_event_2 >> test_sink;

	test_event.fire(0);
	test_sink.expect(0);

	test_event_2.fire(1);
	test_sink.expect(1);
}

//one event source can be connected to several sinks
BOOST_AUTO_TEST_CASE( split_events )
{
	pure::event_source<int> test_event{};
	pure::sink_fixture<int> test_sink_1;
	pure::sink_fixture<int> test_sink_2;

	test_event >> test_sink_1;
	test_event >> test_sink_2;

	test_event.fire(2);
	test_sink_1.expect(2);
	test_sink_2.expect(2);
}

//events can be sent between event_sources and event_sink
BOOST_AUTO_TEST_CASE( in_port )
{
	int test_value{0};

	auto test_writer = [&](int i) {test_value = i;};

	pure::event_sink<int> in_port(test_writer);
	pure::event_source<int> test_event{};

	test_event >> in_port;
	test_event.fire(1);
	BOOST_CHECK_EQUAL(test_value, 1);

	//test void event
	auto write_999 = [&]() {test_value = 999;};

	pure::event_sink<void> void_in(write_999);
	pure::event_source<void> void_out{};
	void_out >> void_in;
	void_out.fire();
	BOOST_CHECK_EQUAL(test_value, 999);
}

//lambdas (and other functors) can serve as sinks for events
BOOST_AUTO_TEST_CASE( lambda )
{
	int test_value {0};

	auto write_666 = [&]() {test_value = 666;};
	pure::event_source<void> void_out_2{};
	void_out_2 >> write_666;
	void_out_2.fire();
	BOOST_CHECK_EQUAL(test_value, 666);
}

namespace
{
template<class T>
void test_connection(const T& connection)
{
	int storage{0};
	pure::event_source<int> a;
	pure::event_sink<int> d{[](int in){ BOOST_CHECK_EQUAL(in, 3);}};
	auto c = [&](int i) { storage = i; return i; };
	auto b = [](int i) { return i + 1; };

	connection(a,b,c,d);

	a.fire(2);
	BOOST_CHECK_EQUAL(storage, 3);
}
}

/**
 * Confirm that connecting ports and connectables
 * does not depend on any particular order.
 */
BOOST_AUTO_TEST_CASE( associativity )
{
	test_connection([](auto& a, auto& b, auto& c, auto& d)
	{
		a >> b >> c >> d;
	});

	test_connection([](auto& a, auto& b, auto& c, auto& d)
	{
		(a >> b) >> (c >> d);
	});

	test_connection([](auto& a, auto& b, auto& c, auto& d)
	{
		a >> ((b >> c) >> d);
	});

	test_connection([](auto& a, auto& b, auto& c, auto& d)
	{
		(a >> (b >> c)) >> d;
	});
}

namespace
{
template<class operation>
struct sink_t
{
	using result_t = void ;
	template <class T>
	void operator()(T&& in) { op(std::forward<T>(in)); }

	operation op;
};

template<class operation>
auto sink(const operation& op )
{
	return sink_t<operation>{op};
}
}

//polymorphic lambdas and functors with overloaded call operators work as well.
BOOST_AUTO_TEST_CASE( test_polymorphic_lambda )
{
	int test_value{0};

	pure::event_source<int> p{};
	auto write = sink([&](auto in) {test_value = in;});

	static_assert(is_passive_sink<decltype(write)>{}, "");

	p >> write;
	BOOST_CHECK_EQUAL(test_value, 0);
	p.fire(4);
	BOOST_CHECK_EQUAL(test_value, 4);
}

BOOST_AUTO_TEST_CASE(test_sink_has_callback)
{
	static_assert(has_register_function<pure::event_sink<void>>(0),
				"type is defined with ability to register a callback");
}

namespace
{
template <class T>
struct disconnecting_event_sink : public pure::event_sink<T>
{
	disconnecting_event_sink() :
		pure::event_sink<T>(
			[&](T in){
				*storage = in;
			}
		)
	{
	}

	std::shared_ptr<T> storage = std::make_shared<T>();
};
}

BOOST_AUTO_TEST_CASE(test_sink_deleted_callback)
{
	disconnecting_event_sink<int> test_sink1{};

	{
		pure::event_source<int> test_source{};

		disconnecting_event_sink<int> test_sink4{};
		test_source >> test_sink1;
		test_source.fire(5);
		BOOST_CHECK_EQUAL(*(test_sink1.storage), 5);

		{
			disconnecting_event_sink<int> test_sink2{};
			disconnecting_event_sink<int> test_sink3{};
			test_source >> test_sink2;
			test_source >> test_sink3;
			test_source.fire(6);
			BOOST_CHECK_EQUAL(*(test_sink2.storage), 6);
			BOOST_CHECK_EQUAL(*(test_sink3.storage), 6);

			test_source >> test_sink4;
			test_source.fire(7);
			BOOST_CHECK_EQUAL(*(test_sink4.storage), 7);

			BOOST_CHECK_EQUAL(test_source.nr_connected_handlers(), 4);
		}

		BOOST_CHECK_EQUAL(test_source.nr_connected_handlers(), 2);

		// this primarily checks, that no exception is thrown
		// since the connections from test_source to sink1-3 are deleted.
		test_source.fire(8);
		BOOST_CHECK_EQUAL(*(test_sink4.storage), 8);
	}
}

BOOST_AUTO_TEST_CASE(test_delete_with_lambda_in_connection)
{
	disconnecting_event_sink<int> test_sink{};

	pure::event_source<int> test_source{};

	(test_source >> [](int i){ return i+1; }) >> test_sink;

	{
		disconnecting_event_sink<int> test_sink_2{};
		test_source >> ([](int i){ return i+1; }
				>> [](int i){ return i+1; })
				>> test_sink_2;
				test_source.fire(10);
				BOOST_CHECK_EQUAL(*(test_sink_2.storage), 12);
				BOOST_CHECK_EQUAL(test_source.nr_connected_handlers(), 2);
	}

	BOOST_CHECK_EQUAL(test_source.nr_connected_handlers(), 1);

	test_source.fire(11);
	BOOST_CHECK_EQUAL(*(test_sink.storage), 12);
}

BOOST_AUTO_TEST_CASE(test_connect_after_move_of_active)
{
	pure::event_source<int> p;
	pure::event_source<int> p2 = std::move(p);
	bool fired{false};
	pure::event_sink<int> sink([&](int v) { fired = true; BOOST_CHECK_EQUAL(v, 99); });
	p2 >> sink;
	p2.fire(99);
	BOOST_CHECK(fired);
}

BOOST_AUTO_TEST_CASE(test_connect_after_move_of_passive)
{
	pure::event_source<int> src{};
	int sink_val{0};
	int ctr{0};
	pure::event_sink<int> sink1{[](int){}};
	{
		pure::event_sink<int> sink2{[&](int v) { ++ctr; sink_val = v; }};
		sink1 = std::move(sink2);
		src >> sink1;
		src >> sink2;
		// after the block ends sink2 should be disconnected
	}
	src.fire(99);
	BOOST_CHECK_EQUAL(sink_val, 99);
	BOOST_CHECK_EQUAL(ctr, 1);
}

BOOST_AUTO_TEST_CASE(test_move_active_after_connect)
{
	pure::event_source<int> p{};
	bool fired{false};
	pure::event_sink<int> sink([&](int v) { fired = true; BOOST_CHECK_EQUAL(v, 99); });
	p >> sink;
	pure::event_source<int> p2 = std::move(p);
	p2.fire(99);
	BOOST_CHECK(fired);
}

BOOST_AUTO_TEST_CASE(lambda_as_sink)
{
	pure::event_source<int> src{};
	int called{0};
	auto lambda = [&](auto v) {
		BOOST_CHECK_EQUAL(v, 99); called++;
	};
	auto mid = [&](auto v) {
		BOOST_CHECK_EQUAL(v, 99);
		called++;
		return v;
	};
	src >> mid >> mid >> std::move(lambda);
	src.fire(99);
	BOOST_CHECK_EQUAL(called, 3);
	bool is_passive_sink_for = fc::is_passive_sink_for<decltype(lambda), int>{};
	BOOST_CHECK(is_passive_sink_for);
}

namespace
{
struct strongly_typed
{
	explicit strongly_typed(double) {}
};
}

BOOST_AUTO_TEST_CASE( type_changing_lambdas )
{
	pure::event_source<double> src{};
	bool called_1{false};
	bool called_2{false};
	src >> [&](double d) {
		called_1 = true;
		return strongly_typed{d};
	} >> [&](strongly_typed) {
		called_2 = true;
	};
	src.fire(1.0);
	BOOST_CHECK(called_1);
	BOOST_CHECK(called_2);
}

BOOST_AUTO_TEST_CASE( type_changing_lambda_with_void )
{
	pure::event_source<void> src{};
	bool called_1{false};
	bool called_2{false};
	src >> [&]() {
		called_1 = true;
		return strongly_typed{1.0};
	} >> [&](strongly_typed) {
		called_2 = true;
	};
	src.fire();
	BOOST_CHECK(called_1);
	BOOST_CHECK(called_2);
}

BOOST_AUTO_TEST_SUITE_END()
