#include <boost/test/unit_test.hpp>

#include <nodes/generic.hpp>


using namespace fc;

BOOST_AUTO_TEST_SUITE(test_generic_nodes)

BOOST_AUTO_TEST_CASE(test_transform)
{
	auto multiply = transform([](int a, int b){ return a * b;});

	[](){ return 3; } >> multiply.param;

	BOOST_CHECK_EQUAL(multiply(2), 6);

	auto add = transform([](int a, int b){ return a + b;});

	auto con = [](){return 4;} >> add >> [](int i) { return i+1; };
	[](){ return 3; } >> add.param;

	BOOST_CHECK_EQUAL(con(), 8);
}

BOOST_AUTO_TEST_CASE(test_n_ary_switch_state)
{
	auto source_1 = [](){return 1;};
	auto source_2 = [](){return 2;};

	n_ary_switch<int, state_tag> test_switch;
	size_t port = 0;
	auto config = [&port](){return port; };

	source_1 >> test_switch.in(0);
	source_2 >> test_switch.in(1);
	config >> test_switch.control();

	BOOST_CHECK_EQUAL(test_switch.out()(), 1);

	port = 1; //change switch to second port
	BOOST_CHECK_EQUAL(test_switch.out()(), 2);
}

BOOST_AUTO_TEST_CASE(test_n_ary_switch_events)
{
	event_out_port<int> source_1;
	event_out_port<int> source_2;

	n_ary_switch<int, event_tag> test_switch;
	size_t port = 0;
	auto config = [&port](){return port; };
	int check = 0;
	auto test_writer = [&check](int i){check = i;};

	source_1 >> test_switch.in(0);
	source_2 >> test_switch.in(1);
	config >> test_switch.control();
	test_switch.out() >> test_writer;

	source_2.fire(2); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(check, 0);
	source_1.fire(1); // tick forwarded source
	BOOST_CHECK_EQUAL(check, 1);

	port = 1;
	check = 0; //reset check

	source_1.fire(1); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(check, 0);
	source_2.fire(2); // tick forwarded source
	BOOST_CHECK_EQUAL(check, 2);
}


BOOST_AUTO_TEST_SUITE_END()
