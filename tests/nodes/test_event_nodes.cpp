#include <boost/test/unit_test.hpp>

#include <flexcore/extended/nodes/event_nodes.hpp>


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

BOOST_AUTO_TEST_CASE(test_gate_with_void_token)
{
	gate_with_control<void> test_gate;

	int test_val = 0;
	bool control_val = false;
	auto sink = [&test_val](void){ test_val++; };
	auto control = [&control_val](){ return control_val; };

	test_gate.out() >> sink;
	control >> test_gate.in_control();
	BOOST_CHECK_EQUAL(test_val, 0);

	test_gate.in()();
	BOOST_CHECK_EQUAL(test_val, 0);

	control_val = true;
	test_gate.in()();
	BOOST_CHECK_EQUAL(test_val, 1);

	test_val = 0;
	control_val = false;
	test_gate.in()();
	BOOST_CHECK_EQUAL(test_val, 0);
}

BOOST_AUTO_TEST_CASE(test_pair_splitter_joiner)
{
	fc::pair_joiner<int, int> joiner;
	fc::pair_splitter<int, int> splitter;

	constexpr int key1 = 1;
	constexpr int key2 = 42;
	constexpr int unused = 666;

	int test_val_1{0};
	int test_val_2{0};

	auto sink1 = [&test_val_1](int in){ test_val_1 = in; };
	auto sink2 = [&test_val_2](int in){ test_val_2 = in; };

	joiner.out() >> splitter.in();

	splitter.out(key1) >> sink1;
	splitter.out(key2) >> sink2;

	joiner.in(key1)(1);

	BOOST_CHECK_EQUAL(test_val_1, 1);
	BOOST_CHECK_EQUAL(test_val_2, 0);

	joiner.in(key2)(2);

	BOOST_CHECK_EQUAL(test_val_1, 1);
	BOOST_CHECK_EQUAL(test_val_2, 2);

	joiner.in(unused)(2);

	//expect unchanged, since no port is connected to key "unused"
	BOOST_CHECK_EQUAL(test_val_1, 1);
	BOOST_CHECK_EQUAL(test_val_2, 2);

}

BOOST_AUTO_TEST_SUITE_END()
