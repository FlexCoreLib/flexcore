#include <boost/test/unit_test.hpp>

#include <nodes/event_nodes.hpp>


using namespace fc;

BOOST_AUTO_TEST_SUITE(test_event_nodes)

BOOST_AUTO_TEST_CASE(test_gate_with_predicate)
{
	auto test_gate = gate<int>([](int i){ return i > 0;});

	int test_val = 0;
	auto sink = [&test_val](int i){ test_val = i; };

	test_gate.out() >> sink;
	BOOST_CHECK_EQUAL(test_val, 0);

	test_gate.in()(-1);
	BOOST_CHECK_EQUAL(test_val, 0);

	test_gate.in()(1);
	BOOST_CHECK_EQUAL(test_val, 1);
}

BOOST_AUTO_TEST_CASE(test_gate_with_control)
{
	gate_with_control<int> test_gate;

	int test_val = 0;
	bool control_val = false;
	auto sink = [&test_val](int i){ test_val = i; };
	auto control = [&control_val](){ return control_val; };

	test_gate.out() >> sink;
	control >> test_gate.in_control();
	BOOST_CHECK_EQUAL(test_val, 0);

	test_gate.in()(1);
	BOOST_CHECK_EQUAL(test_val, 0);

	control_val = true;
	test_gate.in()(1);
	BOOST_CHECK_EQUAL(test_val, 1);

	test_val = 0;
	control_val = false;
	test_gate.in()(1);
	BOOST_CHECK_EQUAL(test_val, 0);
}


BOOST_AUTO_TEST_SUITE_END()
