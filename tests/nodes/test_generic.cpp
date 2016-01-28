#include <boost/test/unit_test.hpp>

#include <nodes/generic.hpp>
#include <ports/events/event_sink_with_queue.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_generic_nodes)

BOOST_AUTO_TEST_CASE(test_transform)
{
	auto multiply = transform( [](int a, int b){ return a * b;});

	pure::state_source_with_setter<int> three(3);
	three >> multiply.param;

	BOOST_CHECK_EQUAL(multiply(2), 6);

	auto add = transform( [](int a, int b){ return a + b;});

	auto con = [](){return 4;} >> add >> [](int i) { return i+1; };
	three >> add.param;

	BOOST_CHECK_EQUAL(con(), 8);
}

BOOST_AUTO_TEST_CASE(test_n_ary_switch_state)
{
	root_node root;
	state_source_with_setter<int> one(&root, 1);
	state_source_with_setter<int> two(&root, 2);

	auto test_switch = root.make_child<n_ary_switch<int, state_tag>>();
	state_source_with_setter<size_t> config(&root, 0);

	one >> test_switch->in(0);
	two >> test_switch->in(1);
	config >> test_switch->control();

	BOOST_CHECK_EQUAL(test_switch->out()(), 1);

	config.access() = 1; //change switch to second port
	BOOST_CHECK_EQUAL(test_switch->out()(), 2);
}

BOOST_AUTO_TEST_CASE(test_n_ary_switch_events)
{
	root_node root;
	event_source<int> source_1(&root);
	event_source<int> source_2(&root);
	auto test_switch = root.make_child<n_ary_switch<int, event_tag>>();
	state_source_with_setter<size_t> config(&root, 0);
	event_sink_queue<int> buffer(&root);

	static_assert(not is_active_sink   <event_source<int>>::value, "");
	static_assert(    is_active_source <event_source<int>>::value, "");
	static_assert(not is_passive_sink  <event_source<int>>::value, "");
	static_assert(not is_passive_source<event_source<int>>::value, "");
	static_assert(not is_active_sink   <decltype(test_switch->in(0))>::value, "");
	static_assert(not is_active_source <decltype(test_switch->in(0))>::value, "");
	static_assert(    is_passive_sink  <decltype(test_switch->in(0))>::value, "");
	static_assert(not is_passive_source<decltype(test_switch->in(0))>::value, "");

	source_1 >> test_switch->in(0);
	source_2 >> test_switch->in(1);
	config >> test_switch->control();
	test_switch->out() >> buffer;

	source_2.fire(2); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(buffer.empty(), true);
	source_1.fire(1); // tick forwarded source
	BOOST_CHECK_EQUAL(buffer.get(), 1);
	BOOST_CHECK_EQUAL(buffer.empty(), true);

	config.access() = 1;

	source_1.fire(1); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(buffer.empty(), true);
	source_2.fire(2); // tick forwarded source
	BOOST_CHECK_EQUAL(buffer.get(), 2);
	BOOST_CHECK_EQUAL(buffer.empty(), true);
}

BOOST_AUTO_TEST_SUITE_END()
