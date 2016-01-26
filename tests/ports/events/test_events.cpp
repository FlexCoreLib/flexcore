#include <boost/test/unit_test.hpp>

#include "ports/events/event_sinks.hpp"
#include "ports/events/event_sources.hpp"
#include "event_sink_with_queue.hpp"
#include "core/connection.hpp"

using namespace fc;

namespace fc
{
namespace pure
{

template<class T>
struct event_sink_value
{
	void operator()(T in) { *storage = in; }
	std::shared_ptr<T> storage = std::make_shared<T>();
};

template<class T>
struct event_sink_vector
{
	void operator()(T in) {	storage->push_back(in);	}
	std::shared_ptr<std::vector<T>> storage = std::make_shared<std::vector<T>>();
};

} // namespace pure

template<class T> struct is_passive_sink<pure::event_sink_value<T>> : public std::true_type {};
template<class T> struct is_passive_sink<pure::event_sink_vector<T>> : public std::true_type {};

} // namespace fc

namespace
{

/**
 * \brief Node for calculating the number of elements in a range
 */
struct range_size
{
public:
	range_size()
		: out()
	{}
	pure::event_source<int> out;

	auto in()
	{
		return ::fc::pure::make_event_sink_tmpl( [this](auto event)
		{
			size_t elems = std::distance(std::begin(event), std::end(event));
			this->out.fire(static_cast<int>(elems));
		} );
	}
};

/**
 * Helper class for testing event_in_port_tmpl
 */
class generic_input_node
{
public:
	generic_input_node() : value() {}

	/*
	 * Define a getter for the port named "in" and
	 * Declare a member function to be called from the port.
	 * The token type is available as "event_t" and the token as "event".
	 */
	auto in()
	{
		return ::fc::pure::make_event_sink_tmpl( [this](auto event)
		{
			value = event;
		} );
	}

	int value;
};

} // unnamed namespace

BOOST_AUTO_TEST_SUITE(test_events)

BOOST_AUTO_TEST_CASE( test_event_in_port_tmpl )
{
	pure::event_source<int> src_int;
	pure::event_source<double> src_double;
	generic_input_node to;

	src_int >> to.in();
	src_double >> to.in();

	src_int.fire(2);
	BOOST_CHECK_EQUAL(to.value, 2);
	src_int.fire(4.1);
	BOOST_CHECK_EQUAL(to.value, 4);
}

BOOST_AUTO_TEST_CASE( connections )
{
	static_assert(is_active<pure::event_source<int>>{},
			"event_out_port is active by definition");
	static_assert(is_passive<pure::event_sink<int>>{},
			"event_in_port is passive by definition");
	static_assert(!is_active<pure::event_sink<int>>{},
			"event_in_port is not active by definition");
	static_assert(!is_passive<pure::event_source<int>>{},
			"event_out_port is not passive by definition");

	pure::event_source<int> test_event;
	pure::event_sink_value<int> test_handler;


	connect(test_event, test_handler);
	test_event.fire(1);
	BOOST_CHECK_EQUAL(*(test_handler.storage), 1);


	auto tmp_connection = test_event >> [](int i){return ++i;};
	static_assert(is_instantiation_of<
			detail::active_connection_proxy, decltype(tmp_connection)>{},
			"active port connected with standard connectable gets proxy");
	tmp_connection >> test_handler;

	test_event.fire(1);
	BOOST_CHECK_EQUAL(*(test_handler.storage), 2);

	auto incr = [](int i){return ++i;};
	test_event >> incr >> incr >> incr >> test_handler;
	test_event.fire(1);
	BOOST_CHECK_EQUAL(*(test_handler.storage), 4);
}


BOOST_AUTO_TEST_CASE( queue_sink )
{
	auto inc = [](int i) { return i + 1; };

	pure::event_source<int> source;
	pure::event_sink_queue<int> sink;
	source >> inc >> sink;
	source.fire(4);
	BOOST_CHECK_EQUAL(sink.empty(), false);
	int received = sink.get();
	BOOST_CHECK_EQUAL(received, 5);
	BOOST_CHECK_EQUAL(sink.empty(), true);
}

BOOST_AUTO_TEST_CASE( merge_events )
{
	pure::event_source<int> test_event;
	pure::event_source<int> test_event_2;
	pure::event_sink_vector<int> test_handler;

	test_event >> test_handler;
	test_event_2 >> test_handler;

	test_event.fire(0);
	BOOST_CHECK_EQUAL(test_handler.storage->size(), 1);
	BOOST_CHECK_EQUAL(test_handler.storage->back(), 0);

	test_event_2.fire(1);

	BOOST_CHECK_EQUAL(test_handler.storage->size(), 2);
	BOOST_CHECK_EQUAL(test_handler.storage->front(), 0);
	BOOST_CHECK_EQUAL(test_handler.storage->back(), 1);

}

BOOST_AUTO_TEST_CASE( split_events )
{
	pure::event_source<int> test_event;
	pure::event_sink_value<int> test_handler_1;
	pure::event_sink_value<int> test_handler_2;

	test_event >> test_handler_1;
	test_event >> test_handler_2;

	test_event.fire(2);
	BOOST_CHECK_EQUAL(*(test_handler_1.storage), 2);
	BOOST_CHECK_EQUAL(*(test_handler_2.storage), 2);
}

BOOST_AUTO_TEST_CASE( in_port )
{
	int test_value = 0;

	auto test_writer = [&](int i) {test_value = i;};

	pure::event_sink<int> in_port(test_writer);
	pure::event_source<int> test_event;

	test_event >> in_port;
	test_event.fire(1);
	BOOST_CHECK_EQUAL(test_value, 1);


	//test void event
	auto write_999 = [&]() {test_value = 999;};


	pure::event_sink<void> void_in(write_999);
	pure::event_source<void> void_out;
	void_out >> void_in;
	void_out.fire();
	BOOST_CHECK_EQUAL(test_value, 999);
}

BOOST_AUTO_TEST_CASE( test_event_out_port )
{
	range_size get_size;
	int storage = 0;
	get_size.out >> [&](int i) { storage = i; };

	get_size.in()(std::list<float>{1., 2., .3});
	BOOST_CHECK_EQUAL(storage, 3);

	get_size.in()(std::vector<int>{0, 1});
	BOOST_CHECK_EQUAL(storage, 2);
}

BOOST_AUTO_TEST_CASE( lambda )
{
	int test_value = 0;

	auto write_666 = [&]() {test_value = 666;};
	pure::event_source<void> void_out_2;
	void_out_2 >> write_666;
	void_out_2.fire();
	BOOST_CHECK_EQUAL(test_value, 666);
}

namespace
{
template<class T>
void test_connection(const T& connection)
{
	int storage = 0;
	pure::event_source<int> a;
	pure::event_sink_queue<int> d;
	auto c = [&](int i) { storage = i; return i; };
	auto b = [](int i) { return i + 1; };

	connection(a,b,c,d);

	a.fire(2);
	BOOST_CHECK_EQUAL(storage, 3);
	BOOST_CHECK_EQUAL(d.get(), 3);
}
}

/**
 * Confirm that connecting ports and connectables
 * does not depend on any particular order.
 */
BOOST_AUTO_TEST_CASE( associativity )
{
	test_connection([](auto a, auto b, auto c, auto d)
	{
		a >> b >> c >> d;
	});

	test_connection([](auto a, auto b, auto c, auto d)
	{
		(a >> b) >> (c >> d);
	});

	test_connection([](auto a, auto b, auto c, auto d)
	{
		a >> ((b >> c) >> d);
	});

	test_connection([](auto a, auto b, auto c, auto d)
	{
		(a >> (b >> c)) >> d;
	});
}

namespace
{
template<class operation>
struct sink_t
{
	typedef void result_t;
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

BOOST_AUTO_TEST_CASE( test_polymorphic_lambda )
{
	int test_value = 0;

	pure::event_source<int> p;
	auto write = sink([&](auto in) {test_value = in;});

	static_assert(is_passive_sink<decltype(write)>{}, "");

	p >> write;
	BOOST_CHECK_EQUAL(test_value, 0);
	p.fire(4);
	BOOST_CHECK_EQUAL(test_value, 4);
}

BOOST_AUTO_TEST_SUITE_END()
