#include <boost/test/unit_test.hpp>

#include <flexcore/extended/ports/node_aware.hpp>
#include <flexcore/pure/pure_ports.hpp>
#include <flexcore/extended/base_node.hpp>
#include <nodes/owning_node.hpp>
#include <pure/sink_fixture.hpp>

#include <boost/mpl/list.hpp>
#include <boost/variant.hpp>

namespace
{
template<class base>
struct useless_mixin : public base
{
	template <class ... args>
	explicit useless_mixin(args&&... base_constructor_args)
		: base(std::forward<args>(base_constructor_args)...)
	{
	}
};

struct node_fixture
{
	fc::tests::owning_node root;

};

using int_event_sink = fc::node_aware<fc::pure::sink_fixture<int>>;
using int_event_source =  fc::node_aware<fc::pure::event_source<int>>;
}

namespace fc
{
template <class T>
struct is_active_source<useless_mixin<T>> : is_active_source<T> {};
template <class T>
struct is_active_sink<useless_mixin<T>> : is_active_sink<T> {};
}

BOOST_FIXTURE_TEST_SUITE(test_parallle_region, node_fixture)


using namespace fc;

BOOST_AUTO_TEST_CASE(test_tick_length_checks)
{
	parallel_region fast_region_1("r1",
		fc::thread::cycle_control::fast_tick);
	parallel_region fast_region_2("r2",
		fc::thread::cycle_control::fast_tick);
	parallel_region medium_region("r3",
		fc::thread::cycle_control::medium_tick);

	node_aware<pure::event_source<int>> port_1(fast_region_1);
	node_aware<pure::event_source<int>> port_2(fast_region_2);
	node_aware<pure::event_source<int>> port_3(medium_region);

	BOOST_CHECK(same_tick_rate(port_1, port_2));
	BOOST_CHECK(!same_tick_rate(port_1, port_3));
	BOOST_CHECK(same_region(port_1, port_1));
	BOOST_CHECK(!same_region(port_1, port_2));
	BOOST_CHECK(!same_tick_rate(port_1, port_3));
}

BOOST_AUTO_TEST_CASE(test_same_region)
{
	int_event_sink test_in(*(root.region()));
	int_event_source test_out(*(root.region()));

	test_out >> test_in;

	test_out.fire(1);
	test_in.expect(1);

	auto tmp = test_out >> [](int i ){ return ++i;};

	static_assert(!is_passive_source<decltype(tmp)>{}, "");
	static_assert(!is_passive_sink  <decltype(tmp)>{}, "");
	static_assert( is_active_source <decltype(tmp)>{}, "");
	static_assert(!is_active_sink   <decltype(tmp)>{}, "");

	std::move(tmp) >> test_in;

	test_out.fire(1);
	test_in.expect(1); //one for first connection
	test_in.expect(2); //another for second with increment
}

namespace
{
template<class source_t, class sink_t, class T>
void check_mixins()
{
	constexpr auto default_tick = thread::cycle_control::slow_tick;
	constexpr auto different_tick = thread::cycle_control::fast_tick;

	auto region_1 = std::make_shared<parallel_region>("r1", default_tick);
	auto region_2 = std::make_shared<parallel_region>("r2", default_tick);
	auto region_3 = std::make_shared<parallel_region>("r3", different_tick);
	tests::owning_node root_1(region_1);
	tests::owning_node root_2(region_2);
	tests::owning_node root_3(region_3);

	T test_value_1{0};
	T test_value_2{0};
	auto write_param_1 = [&test_value_1](T i) {test_value_1 = i;};
	auto write_param_2 = [&test_value_2](T i) {test_value_2 = i;};
	sink_t test_in_1(*(root_2.region()), write_param_1);
	sink_t test_in_2(*(root_3.region()), write_param_2);
	source_t test_out(*(root_1.region()));

	test_out >> test_in_1;
	test_out >> test_in_2;
	static_assert(is_active<source_t>{}, "not active source.");
	static_assert(is_connectable<sink_t&>{}, "no connectable sink.");
	static_assert(is_passive<sink_t>{}, "no passive sink.");

	BOOST_CHECK_EQUAL(test_value_1, T{0});

	// after firing, both test values should be zero (passive regions did not receive)
	test_out.fire(T{1});
	BOOST_CHECK_EQUAL(test_value_1, T{0});
	BOOST_CHECK_EQUAL(test_value_2, T{0});

	// after the active regions's switch tick, both should still be zero
	region_1->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(test_value_1, T{0});
	BOOST_CHECK_EQUAL(test_value_2, T{0});

	// after the passive regions's work tick,
	// the region with the same tick length should have received something
	region_2->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value_1, T{1});

	// after switching and working, the other region should also receive something
	region_3->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(test_value_2, T{0});
	region_3->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value_2, T{1});
}
}

// test region transition with different types, to make sure we don't depend on int.
// boost variant for types that are not trivially copyable
// and have a constructor which takes an initializer list
using token_types = boost::mpl::list<int, float, boost::variant<int>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_different_region, T, token_types)
{
	using no_mixin_sink = node_aware<pure::event_sink<T>>;
	using no_mixin_source = node_aware<pure::event_source<T>>;
	check_mixins<no_mixin_source, no_mixin_sink, T>();

	using test_mixin_sink = useless_mixin<node_aware<pure::event_sink<T>>>;
	using test_mixin_source = useless_mixin<node_aware<pure::event_source<T>>>;
	check_mixins<test_mixin_source, test_mixin_sink, T>();
}

BOOST_AUTO_TEST_CASE(test_void_event)
{
	parallel_region region_1{"r1",
		fc::thread::cycle_control::fast_tick};
	parallel_region region_2{"r2",
		fc::thread::cycle_control::fast_tick};

	node_aware<pure::event_source<void>> source{region_1};
	bool written{false};
	node_aware<pure::event_sink<void>> sink{region_1, [&written](){written=true;}};
	node_aware<pure::event_sink<void>> sink2{region_2, [&written](){written=true;}};

	source >> sink;
	source >> sink2;

	BOOST_CHECK(!written);
	source.fire();
	BOOST_CHECK(written);
	written = false;

	region_1.ticks.switch_buffers();
	BOOST_CHECK(!written);
	region_2.ticks.switch_buffers();
	BOOST_CHECK(!written);
	region_2.ticks.in_work()();
	BOOST_CHECK(written);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_traits, T, token_types)
{
	using full_state_sink = state_sink<T>;
	using full_state_source = state_source<T>;
	using full_event_sink = event_sink<T>;
	using full_event_source = event_source<T>;

	BOOST_CHECK( is_active_sink   <full_state_sink>{});
	BOOST_CHECK(!is_active_source <full_state_sink>{});
	BOOST_CHECK(!is_passive_sink  <full_state_sink>{});
	BOOST_CHECK(!is_passive_source<full_state_sink>{});

	BOOST_CHECK(!is_active_sink   <full_state_source>{});
	BOOST_CHECK(!is_active_source <full_state_source>{});
	BOOST_CHECK(!is_passive_sink  <full_state_source>{});
	BOOST_CHECK( is_passive_source<full_state_source>{});

	BOOST_CHECK(!is_active_sink   <full_event_sink>{});
	BOOST_CHECK(!is_active_source <full_event_sink>{});
	BOOST_CHECK( is_passive_sink  <full_event_sink>{});
	BOOST_CHECK(!is_passive_source<full_event_sink>{});

	BOOST_CHECK(!is_active_sink   <full_event_source>{});
	BOOST_CHECK( is_active_source <full_event_source>{});
	BOOST_CHECK(!is_passive_sink  <full_event_source>{});
	BOOST_CHECK(!is_passive_source<full_event_source>{});
}

BOOST_AUTO_TEST_CASE(test_connectable_in_between)
{
	using test_in_port = node_aware<pure::event_sink<int>> ;

	auto region_1 = std::make_shared<parallel_region>("r1",
			fc::thread::cycle_control::fast_tick);
	auto region_2 = std::make_shared<parallel_region>("r2",
			fc::thread::cycle_control::fast_tick);
	tests::owning_node root_1(region_1);
	tests::owning_node root_2(region_2);

	int test_value{0};
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(*(root_2.region()), write_param);
	int_event_source test_out(*(root_1.region()));

	test_out >> [](auto i){ return i+1;} >> test_in;

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	//since we have a region transition, we need a switch tick
	BOOST_CHECK_EQUAL(test_value, 0);

	region_1->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 2);

	// test more than one lambda in between
	auto region_3 = std::make_shared<parallel_region>("r3",
			fc::thread::cycle_control::fast_tick);
	tests::owning_node root_3(region_3);

	test_in_port test_in_2(*(root_3.region()), write_param);
	test_out
			>> [](int i){ return i+1;}
			>> [](int i){ return i*2;}
			>> test_in_2;

	test_out.fire(1);
	region_1->ticks.switch_buffers();
	region_2->ticks.in_work()();
	// wrong region ticked, expect no change
	BOOST_CHECK_EQUAL(test_value, 2);
	region_3->ticks.switch_buffers();

	region_3->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 4);
}

BOOST_AUTO_TEST_CASE(test_multiple_connectable_in_between)
{
	using test_in_port = node_aware<pure::event_sink<int>> ;

	auto region_1 = std::make_shared<parallel_region>("r1",
			fc::thread::cycle_control::fast_tick);
	auto region_2 = std::make_shared<parallel_region>("r2",
			fc::thread::cycle_control::fast_tick);
	tests::owning_node root_1(region_1);
	tests::owning_node root_2(region_2);

	int test_value{0};
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(*(root_2.region()), write_param);
	int_event_source test_out(*(root_1.region()));

	auto inc = [](int i){ return i + 1; };

	(test_out >> inc) >> inc >> (inc >> test_in);

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	//since we have a region transition, we need a switch tick
	BOOST_CHECK_EQUAL(test_value, 0);

	region_1->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 4);

	// check the same for states
	using test_in_state = node_aware<pure::state_sink<int>> ;
	using test_out_state = node_aware<pure::state_source<int>>;

	test_out_state state_out{*(root_1.region()), [](){ return 1; }};
	test_in_state state_in{*(root_2.region())};

	(state_out >> inc) >> inc >> (inc >> state_in);
	//                                 ^^^ buffer is here

	BOOST_CHECK_EQUAL(state_in.get(), 0);
	region_1->ticks.in_work()();
	BOOST_CHECK_EQUAL(state_in.get(), 0);
	region_1->ticks.switch_buffers(); // moved to middle buffer
	BOOST_CHECK_EQUAL(state_in.get(), 0);
	region_2->ticks.switch_buffers(); // moved to outgoing buffer
	BOOST_CHECK_EQUAL(state_in.get(), 4);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_state_transition, T, token_types)
{
	const auto default_tick = thread::cycle_control::slow_tick;
	const auto different_tick = thread::cycle_control::fast_tick;
	using test_in_port = node_aware<pure::state_sink<T>>;
	using test_out_port = node_aware<pure::state_source<T>>;
	auto region_1 = std::make_shared<parallel_region>("r1", default_tick);
	auto region_2 = std::make_shared<parallel_region>("r2", default_tick);
	auto region_3 = std::make_shared<parallel_region>("r3", different_tick);
	tests::owning_node root_1(region_1);
	tests::owning_node root_2(region_2);
	tests::owning_node root_3(region_3);

	test_out_port source(*(root_1.region()), [](){ return T{1}; });
	test_in_port sink_1(*(root_2.region()));
	test_in_port sink_2(*(root_3.region()));

	static_assert(std::is_same<T,
			result_of_t<test_out_port>>{},
			"return value of source is defined to be int");

	source >> [](auto in){ return in;} >> sink_1;
	source >> [](auto in){ return in;} >> sink_2;

	BOOST_CHECK_EQUAL(sink_1.get(), T{0});

	region_1->ticks.in_work()();
	BOOST_CHECK_EQUAL(sink_1.get(), T{0});
	BOOST_CHECK_EQUAL(sink_2.get(), T{0});

	// Switching active side buffers, sufficient for regions with same tick rate
	region_2->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(sink_1.get(), T{1});

	// For differing tick rates, both must be switched
	region_1->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(sink_2.get(), T{0});
	region_3->ticks.switch_buffers();
	BOOST_CHECK_EQUAL(sink_2.get(), T{1});
}

BOOST_AUTO_TEST_CASE(test_state_same_region)
{
	using test_in_port = node_aware<pure::state_sink<int>>;
	using test_out_port = node_aware<pure::state_source<int>>;

	test_out_port source(*(root.region()), [](){ return 1; });
	test_in_port sink(*(root.region()));

	source >> sink;

	//expect result to be available immediately without switch tick,
	//since we are in the same region
	BOOST_CHECK_EQUAL(sink.get(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
