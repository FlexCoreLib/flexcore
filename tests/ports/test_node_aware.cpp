#include <boost/test/unit_test.hpp>

#include <ports/ports.hpp>
#include <ports/region_aware.hpp>

using namespace fc;

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

	auto connection = test_out >> test_in;
	static_assert(is_instantiation_of<node_aware_connection, decltype(connection)>::value,
			"connection should be node_aware_connection");

	BOOST_CHECK_EQUAL(test_value, 0);
	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_value, 1);

	auto tmp = test_out >> [](int i ){ return ++i;};
	tmp >> test_in;

	test_out.fire(1);
	BOOST_CHECK_EQUAL(test_value, 2);
}

namespace unit_test
{
class parallel_tester
{
public:
	static void switch_tick(std::shared_ptr<parallel_region> region)
	{
		region->ticks.in_switch_buffers()();
	}
	static void work_tick(std::shared_ptr<parallel_region> region)
	{
		region->ticks.in_work()();
	}
};

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
	std::cout << "TEST SEND EVENT!\n";
	test_out.fire(1);
	//since we have a region transition, we need a switch tick
	BOOST_CHECK_EQUAL(test_value, 0);

	std::cout << "TEST SWITCH!\n";
	::unit_test::parallel_tester::switch_tick(region_1);
	BOOST_CHECK_EQUAL(test_value, 0);
	std::cout << "TEST WORK!\n";
	::unit_test::parallel_tester::work_tick(region_2);
	BOOST_CHECK_EQUAL(test_value, 1);
}
