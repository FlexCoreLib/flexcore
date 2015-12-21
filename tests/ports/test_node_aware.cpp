#include <boost/test/unit_test.hpp>

#include <ports/ports.hpp>
#include <ports/node_aware.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_parallle_region);

BOOST_AUTO_TEST_CASE(test_region_aware_node)
{
	root_node root;
	typedef node_aware<pure::event_sink<int>> test_in_port;

	int test_value = 0;
	auto write_param = [&](int i) {test_value = i;};
	test_in_port test_in(&root, write_param);

	BOOST_CHECK_EQUAL(test_value, 0);
	test_in(1);
	BOOST_CHECK_EQUAL(test_value, 1);
}


BOOST_AUTO_TEST_CASE(test_same_region)
{
	typedef node_aware<pure::event_sink<int>> test_in_port;
	typedef node_aware<pure::event_source<int>> test_out_port;

	root_node root;

	std::vector<int> test_sink;
	auto write_param = [&](int i) {test_sink.push_back(i);};
	test_in_port test_in(&root, write_param);
	test_out_port test_out(&root);

	static_assert(is_passive_sink<test_in_port>::value, "");
	static_assert(has_result<test_out_port>::value,
			"its an out port, that has result_t defined");

	test_out >> test_in;

	BOOST_CHECK_EQUAL(test_sink.size(), 0);
	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_sink.size(), 1);
	BOOST_CHECK_EQUAL(test_sink.at(0), 1);

	auto tmp = test_out >> [](int i ){ return ++i;};

	static_assert(is_instantiation_of<node_aware, test_in_port>::value, "");
	static_assert(is_instantiation_of<node_aware, decltype(tmp)>::value, "");
	static_assert(not is_active_sink   <test_in_port>::value, "");
	static_assert(not is_active_source <test_in_port>::value, "");
	static_assert(    is_passive_sink  <test_in_port>::value, "");
	static_assert(not is_passive_source<test_in_port>::value, "");
	static_assert(not is_passive_source<decltype(tmp)>::value, "");
	static_assert(not is_passive_sink  <decltype(tmp)>::value, "");
	static_assert(    is_active_source <decltype(tmp)>::value, "");
	static_assert(not is_active_sink   <decltype(tmp)>::value, "");

	tmp >> test_in;

	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_sink.at(1), 1);
	BOOST_CHECK_EQUAL(test_sink.at(2), 2);
}

BOOST_AUTO_TEST_CASE(test_different_region)
{
	typedef node_aware<pure::event_sink<int>> test_in_port;
	typedef node_aware<pure::event_source<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	root_node root_1("1", region_1);
	root_node root_2("2", region_2);

	int test_value = 0;
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(&root_2, write_param);
	test_out_port test_out(&root_1);

	test_out >> test_in;

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	//since we have a region transition, we need a switch tick
	BOOST_CHECK_EQUAL(test_value, 0);

	region_1->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 1);
}

BOOST_AUTO_TEST_CASE(test_connectable_in_between)
{
	typedef node_aware<pure::event_sink<int>> test_in_port;
	typedef node_aware<pure::event_source<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	root_node root_1("1", region_1);
	root_node root_2("2", region_2);

	int test_value = 0;
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(&root_2, write_param);
	test_out_port test_out(&root_1);

	test_out >> [](int i){ return i+1;} >> test_in;

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	//since we have a region transition, we need a switch tick
	BOOST_CHECK_EQUAL(test_value, 0);

	region_1->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(test_value, 0);
	region_2->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 2);

	// test more than one lambda in between
	auto region_3 = std::make_shared<parallel_region>("r3");
	root_node root_3("3", region_3);

	test_in_port test_in_2(&root_3, write_param);
	test_out
			>> [](int i){ return i+1;}
			>> [](int i){ return i*2;}
			>> test_in_2;

	test_out.fire(1);
	region_1->ticks.in_switch_buffers()();
	region_2->ticks.in_work()();
	// wrong region ticked, expect no change
	BOOST_CHECK_EQUAL(test_value, 2);

	region_3->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 4);
}

BOOST_AUTO_TEST_CASE(test_state_transition)
{
	typedef node_aware<pure::state_sink<int>> test_in_port;
	typedef node_aware<pure::state_source_with_setter<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	root_node root_1("1", region_1);
	root_node root_2("2", region_2);

	test_out_port source(&root_1, 1);
	test_in_port sink(&root_2);

	static_assert(is_instantiation_of<node_aware, test_in_port>::value, "");
	static_assert(is_instantiation_of<node_aware, test_out_port>::value, "");
	static_assert(    is_active_sink   <test_in_port>::value, "");
	static_assert(not is_active_source <test_in_port>::value, "");
	static_assert(not is_passive_sink  <test_in_port>::value, "");
	static_assert(not is_passive_source<test_in_port>::value, "");
	static_assert(    is_passive_source<test_out_port>::value, "");
	static_assert(not is_passive_sink  <test_out_port>::value, "");
	static_assert(not is_active_source <test_out_port>::value, "");
	static_assert(not is_active_sink   <test_out_port>::value, "");

	static_assert(std::is_same<int,
			typename result_of<test_out_port>::type>::value,
			"return value of source is defined to be int");
	source >> sink;


	BOOST_CHECK_EQUAL(sink.get(), 0);

	region_1->ticks.in_work()();
	BOOST_CHECK_EQUAL(sink.get(), 0);
	region_2->ticks.in_switch_buffers()();
	BOOST_CHECK_EQUAL(sink.get(), 1);
}

BOOST_AUTO_TEST_CASE(test_state_same_region)
{
	typedef node_aware<pure::state_sink<int>> test_in_port;
	typedef node_aware<pure::state_source_with_setter<int>> test_out_port;
	root_node root;

	test_out_port source(&root, 1);
	test_in_port sink(&root);

	source >> sink;

	//expect result to be available immediately without switch tick,
	//since we are in the same region
	BOOST_CHECK_EQUAL(sink.get(), 1);
}

BOOST_AUTO_TEST_SUITE_END();
