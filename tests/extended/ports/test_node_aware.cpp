#include <boost/test/unit_test.hpp>

#include <flexcore/extended/ports/node_aware.hpp>
#include <flexcore/pure/pure_ports.hpp>
#include <flexcore/extended/base_node.hpp>
#include <nodes/owning_node.hpp>

template<class base>
struct useless_mixin : public base
{
	template <class ... args>
	useless_mixin(args&&... base_constructor_args)
		: base(std::forward<args>(base_constructor_args)...)
	{
	}
};

namespace fc
{
template <class T>
struct is_active_source<useless_mixin<T>> : is_active_source<T> {};
template <class T>
struct is_active_sink<useless_mixin<T>> : is_active_sink<T> {};
}

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_parallle_region)

BOOST_AUTO_TEST_CASE(test_region_aware_node)
{
	tests::owning_node root;
	typedef node_aware<pure::event_sink<int>> test_in_port;

	int test_value = 0;
	auto write_param = [&](int i) {test_value = i;};
	test_in_port test_in(*(root.region()), write_param);

	BOOST_CHECK_EQUAL(test_value, 0);
	test_in(1);
	BOOST_CHECK_EQUAL(test_value, 1);
}


BOOST_AUTO_TEST_CASE(test_same_region)
{
	typedef node_aware<pure::event_sink<int>> test_in_port;
	typedef node_aware<pure::event_source<int>> test_out_port;

	tests::owning_node root;

	std::vector<int> test_sink;
	auto write_param = [&](int i) {test_sink.push_back(i);};
	test_in_port test_in(*(root.region()), write_param);
	test_out_port test_out(*(root.region()));

	static_assert(is_passive_sink<test_in_port>{}, "");
	static_assert(has_result<test_out_port>{},
			"its an out port, that has result_t defined");

	test_out >> test_in;

	BOOST_CHECK_EQUAL(test_sink.size(), 0);
	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_sink.size(), 1);
	BOOST_CHECK_EQUAL(test_sink.at(0), 1);

	auto tmp = test_out >> [](int i ){ return ++i;};

	static_assert(is_instantiation_of<node_aware, test_in_port>{}, "");
	static_assert(! is_active_sink   <test_in_port>{}, "");
	static_assert(! is_active_source <test_in_port>{}, "");
	static_assert(  is_passive_sink  <test_in_port>{}, "");
	static_assert(! is_passive_source<test_in_port>{}, "");
	static_assert(! is_passive_source<decltype(tmp)>{}, "");
	static_assert(! is_passive_sink  <decltype(tmp)>{}, "");
	static_assert(  is_active_source <decltype(tmp)>{}, "");
	static_assert(! is_active_sink   <decltype(tmp)>{}, "");

	std::move(tmp) >> test_in;

	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_sink.at(1), 1);
	BOOST_CHECK_EQUAL(test_sink.at(2), 2);
}

namespace
{
template<class source_t, class sink_t>
void check_mixins()
{
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	tests::owning_node root_1(region_1);
	tests::owning_node root_2(region_2);

	int test_value = 0;
	auto write_param = [&test_value](int i) {test_value = i;};
	sink_t test_in(*(root_2.region()), write_param);
	source_t test_out(*(root_1.region()));

	test_out >> test_in;
	static_assert(is_active<source_t>{}, "not active source.");
	static_assert(is_connectable<sink_t&>{}, "no connectable sink.");
	static_assert(is_passive<sink_t>{}, "no passive sink.");


	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	//since we have a region transition, we need a switch tick
	BOOST_CHECK_EQUAL(test_value, 0);

	region_1->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 1);
}
}

BOOST_AUTO_TEST_CASE(test_different_region)
{
	typedef node_aware<pure::event_sink<int>> no_mixin_sink;
	typedef node_aware<pure::event_source<int>> no_mixin_source;
	check_mixins<no_mixin_source, no_mixin_sink>();

	typedef useless_mixin<node_aware<pure::event_sink<int>>> test_mixin_sink;
	typedef useless_mixin<node_aware<pure::event_source<int>>> test_mixin_source;
	check_mixins<test_mixin_source, test_mixin_sink>();
}

BOOST_AUTO_TEST_CASE(test_void_event)
{
	parallel_region region_1{"r1"};
	parallel_region region_2{"r2"};

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

	region_1.ticks.in_switch_buffers()();
	BOOST_CHECK(!written);
	region_2.ticks.in_switch_buffers()();
	BOOST_CHECK(!written);
	region_2.ticks.in_work()();
	BOOST_CHECK(written);
}

BOOST_AUTO_TEST_CASE(test_traits)
{
	using full_state_sink = state_sink<int>;
	using full_state_source = state_source<int>;
	using full_event_sink = event_sink<int>;
	using full_event_source = event_source<int>;

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
	typedef node_aware<pure::event_sink<int>> test_in_port;
	typedef node_aware<pure::event_source<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	tests::owning_node root_1(region_1);
	tests::owning_node root_2(region_2);

	int test_value = 0;
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(*(root_2.region()), write_param);
	test_out_port test_out(*(root_1.region()));

	test_out >> [](auto i){ return i+1;} >> test_in;

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	//since we have a region transition, we need a switch tick
	BOOST_CHECK_EQUAL(test_value, 0);

	region_1->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 2);

	// test more than one lambda in between
	auto region_3 = std::make_shared<parallel_region>("r3");
	tests::owning_node root_3(region_3);

	test_in_port test_in_2(*(root_3.region()), write_param);
	test_out
			>> [](int i){ return i+1;}
			>> [](int i){ return i*2;}
			>> test_in_2;

	test_out.fire(1);
	region_1->ticks.in_switch_buffers()();
	region_2->ticks.in_work()();
	// wrong region ticked, expect no change
	BOOST_CHECK_EQUAL(test_value, 2);
	region_3->ticks.in_switch_buffers()();

	region_3->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 4);
}

BOOST_AUTO_TEST_CASE(test_multiple_connectable_in_between)
{
	typedef node_aware<pure::event_sink<int>> test_in_port;
	typedef node_aware<pure::event_source<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	tests::owning_node root_1(region_1);
	tests::owning_node root_2(region_2);

	int test_value = 0;
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(*(root_2.region()), write_param);
	test_out_port test_out(*(root_1.region()));

	auto inc = [](int i){ return i + 1; };

	(test_out >> inc) >> inc >> (inc >> test_in);

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	//since we have a region transition, we need a switch tick
	BOOST_CHECK_EQUAL(test_value, 0);

	region_1->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 4);

	// check the same for states
	typedef node_aware<pure::state_sink<int>> test_in_state;
	typedef node_aware<pure::state_source<int>> test_out_state;

	test_out_state state_out{*(root_1.region()), [](){ return 1; }};
	test_in_state state_in{*(root_2.region())};

	(state_out >> inc) >> inc >> (inc >> state_in);
	//                                 ^^^ buffer is here

	BOOST_CHECK_EQUAL(state_in.get(), 0);
	region_1->ticks.in_work()();
	BOOST_CHECK_EQUAL(state_in.get(), 0);
	region_1->ticks.in_switch_buffers()(); // moved to middle buffer
	BOOST_CHECK_EQUAL(state_in.get(), 0);
	region_2->ticks.in_switch_buffers()(); // moved to outgoing buffer
	BOOST_CHECK_EQUAL(state_in.get(), 4);
}

BOOST_AUTO_TEST_CASE(test_state_transition)
{
	typedef node_aware<pure::state_sink<int>> test_in_port;
	typedef node_aware<pure::state_source<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	tests::owning_node root_1(region_1);
	tests::owning_node root_2(region_2);

	test_out_port source(*(root_1.region()), [](){ return 1; });
	test_in_port sink(*(root_2.region()));

	static_assert(is_instantiation_of<node_aware, test_in_port>{}, "");
	static_assert(is_instantiation_of<node_aware, test_out_port>{}, "");
	static_assert(  is_active_sink   <test_in_port>{}, "");
	static_assert(! is_active_source <test_in_port>{}, "");
	static_assert(! is_passive_sink  <test_in_port>{}, "");
	static_assert(! is_passive_source<test_in_port>{}, "");
	static_assert(  is_passive_source<test_out_port>{}, "");
	static_assert(! is_passive_sink  <test_out_port>{}, "");
	static_assert(! is_active_source <test_out_port>{}, "");
	static_assert(! is_active_sink   <test_out_port>{}, "");

	static_assert(std::is_same<int,
			result_of_t<test_out_port>>{},
			"return value of source is defined to be int");
	source >> [](auto in){ return in;} >> sink;


	BOOST_CHECK_EQUAL(sink.get(), 0);

	region_1->ticks.in_work()();
	BOOST_CHECK_EQUAL(sink.get(), 0);
	region_1->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(sink.get(), 0);
	region_2->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(sink.get(), 1);
}

BOOST_AUTO_TEST_CASE(test_state_same_region)
{
	typedef node_aware<pure::state_sink<int>> test_in_port;
	typedef node_aware<pure::state_source<int>> test_out_port;
	tests::owning_node root;

	test_out_port source(*(root.region()), [](){ return 1; });
	test_in_port sink(*(root.region()));

	source >> sink;

	//expect result to be available immediately without switch tick,
	//since we are in the same region
	BOOST_CHECK_EQUAL(sink.get(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
