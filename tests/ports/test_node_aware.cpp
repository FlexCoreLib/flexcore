#include <boost/test/unit_test.hpp>

#include <ports/ports.hpp>
#include "../../src/ports/node_aware.hpp"

using namespace fc;

namespace fc
{
class identidy_node : public node_interface
{
public:
	identidy_node( node_interface* p,
				   std::shared_ptr<region_info> r = std::shared_ptr<region_info>() )
		: node_interface(p, "identity", r)
	{}
	identidy_node( std::shared_ptr<region_info> r = std::shared_ptr<region_info>() )
		: node_interface(std::make_shared<node_interface::forest_t>(), "identity", r)
	{}
};
} // namespace fc

BOOST_AUTO_TEST_SUITE(test_parallle_region);

BOOST_AUTO_TEST_CASE(test_region_aware_node)
{
	auto region = std::make_shared<parallel_region>();
	identidy_node identity(region);
	typedef node_aware<event_in_port<int>> test_in_port;

	int test_value = 0;
	auto write_param = [&](int i) {test_value = i;};
	test_in_port test_in(&identity, write_param);

	BOOST_CHECK_EQUAL(test_value, 0);
	test_in(1);
	BOOST_CHECK_EQUAL(test_value, 1);
}


BOOST_AUTO_TEST_CASE(test_same_region)
{
	typedef node_aware<event_in_port<int>> test_in_port;
	typedef node_aware<event_out_port<int>> test_out_port;


	auto region = std::make_shared<parallel_region>();
	identidy_node identity(region);

	std::vector<int> test_sink;
	auto write_param = [&](int i) {test_sink.push_back(i);};
	test_in_port test_in(&identity, write_param);
	test_out_port test_out(&identity);

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
	static_assert(not is_active_sink<test_in_port>::value, "");
	static_assert(not is_active_source<test_in_port>::value, "");
	static_assert(is_passive_sink<test_in_port>::value, "");
	static_assert(not is_passive_source<test_in_port>::value, "");
	static_assert(not is_passive_source<decltype(tmp)>::value, "");
	static_assert(not is_passive_sink<decltype(tmp)>::value, "");
	static_assert(is_active_source<decltype(tmp)>::value, "");
	static_assert(not is_active_sink<decltype(tmp)>::value, "");

	tmp >> test_in;

	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_sink.at(1), 1);
	BOOST_CHECK_EQUAL(test_sink.at(2), 2);
}

BOOST_AUTO_TEST_CASE(test_different_region)
{
	typedef node_aware<event_in_port<int>> test_in_port;
	typedef node_aware<event_out_port<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	identidy_node identity_1(region_1);
	identidy_node identity_2(region_2);

	int test_value = 0;
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(&identity_2, write_param);
	test_out_port test_out(&identity_1);

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
	typedef node_aware<event_in_port<int>> test_in_port;
	typedef node_aware<event_out_port<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	identidy_node identity_1(region_1);
	identidy_node identity_2(region_2);

	int test_value = 0;
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(&identity_2, write_param);
	test_out_port test_out(&identity_1);

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
	identidy_node identity_3(region_3);

	test_in_port test_in_2(&identity_3, write_param);
	test_out >>
			[](int i){ return i+1;} >>
			[](int i){ return i*2;} >>
			test_in_2;

	test_out.fire(1);
	region_1->ticks.in_switch_buffers()();
	region_2->ticks.in_work()();
//wrong region ticked, expect no change
	BOOST_CHECK_EQUAL(test_value, 2);

	region_3->ticks.in_work()();
	BOOST_CHECK_EQUAL(test_value, 4);
}

BOOST_AUTO_TEST_CASE(test_state_transition)
{
	typedef node_aware<state_sink<int>> test_in_port;
	typedef node_aware<state_source_with_setter<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");
	identidy_node identity_1(region_1);
	identidy_node identity_2(region_2);

	test_out_port source(&identity_1, 1);
	test_in_port sink(&identity_2);

	static_assert(is_instantiation_of<node_aware, test_in_port>::value, "");
	static_assert(is_instantiation_of<node_aware, test_out_port>::value, "");
	static_assert(is_active_sink<test_in_port>::value, "");
//	static_assert(not is_active_source<test_in_port>::value, ""); // FIXME
	static_assert(not is_passive_sink<test_in_port>::value, "");
	static_assert(not is_passive_source<test_in_port>::value, "");
	static_assert(is_passive_source<test_out_port>::value, "");
	static_assert(not is_passive_sink<test_out_port>::value, "");
	static_assert(not is_active_source<test_out_port>::value, "");
	static_assert(not is_active_sink<test_out_port>::value, "");

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
	typedef node_aware<state_sink<int>> test_in_port;
	typedef node_aware<state_source_with_setter<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	identidy_node identity_1(region_1);

	test_out_port source(&identity_1,1);
	test_in_port sink(&identity_1);

	source >> sink;

	//expect result to be available immediately without switch tick,
	//since we are in the same region
	BOOST_CHECK_EQUAL(sink.get(), 1);
}

BOOST_AUTO_TEST_SUITE_END();
