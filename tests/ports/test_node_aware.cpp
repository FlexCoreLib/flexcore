#include <boost/test/unit_test.hpp>

#include <ports/ports.hpp>
#include <ports/region_aware.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_parallle_region);

BOOST_AUTO_TEST_CASE(test_region_aware_node)
{
	typedef region_aware<event_in_port<int>> test_in_port;
	auto region = std::make_shared<parallel_region>();

	int test_value = 0;
	auto write_param = [&](int i) {test_value = i;};
	test_in_port test_in(region, write_param);

	BOOST_CHECK_EQUAL(test_value, 0);
	test_in.operator ()(1);
	BOOST_CHECK_EQUAL(test_value, 1);
}


BOOST_AUTO_TEST_CASE(test_same_region)
{
	typedef region_aware<event_in_port<int>> test_in_port;
	typedef region_aware<event_out_port<int>> test_out_port;
	auto region = std::make_shared<parallel_region>();

	int test_value = 0;
	auto write_param = [&](int i) {test_value = i;};
	test_in_port test_in(region, write_param);
	test_out_port test_out(region);

	static_assert(is_passive_sink<test_in_port>::value, "");
	static_assert(has_result<test_out_port>::value,
			"its an out port, that has result_type defined");

	test_out >> test_in;

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_value, 1);

	auto tmp = test_out >> [](int i ){ return ++i;};
	tmp >> test_in;

	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_value, 2);
}

BOOST_AUTO_TEST_CASE(test_different_region)
{
	typedef region_aware<event_in_port<int>> test_in_port;
	typedef region_aware<event_out_port<int>> test_out_port;
	auto region_1 = std::make_shared<parallel_region>("r1");
	auto region_2 = std::make_shared<parallel_region>("r2");


	int test_value = 0;
	auto write_param = [&test_value](int i) {test_value = i;};
	test_in_port test_in(region_2, write_param);
	test_out_port test_out(region_1);

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

BOOST_AUTO_TEST_SUITE_END();
