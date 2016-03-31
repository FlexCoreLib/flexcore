#include <boost/test/unit_test.hpp>

#include <extended/nodes/generic.hpp>
#include <pure/events/event_sink_with_queue.hpp>

#include "owning_node.hpp"

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_generic_nodes)

BOOST_AUTO_TEST_CASE(test_transform)
{
	auto multiply = transform( [](int a, int b){ return a * b;});

	auto three = [](){ return 3; };
	three >> multiply.param;

	BOOST_CHECK_EQUAL(multiply(2), 6);

	auto add = transform( [](int a, int b){ return a + b;});

	auto con = [](){return 4;} >> add >> [](int i) { return i+1; };
	three >> add.param;

	BOOST_CHECK_EQUAL(con(), 8);
}

BOOST_AUTO_TEST_CASE(test_n_ary_switch_state)
{
	tests::owning_node root;
	state_source<int> one(&root.node(), [](){ return 1; });
	state_source<int> two(&root.node(), [](){ return 2; });

	auto test_switch = root.make_child_named<n_ary_switch<int, state_tag>>("switch");
	size_t switch_param = 0;
	state_source<size_t> config(&root.node(), [&switch_param](){ return switch_param; });

	one >> test_switch->in(0);
	two >> test_switch->in(1);
	config >> test_switch->control();

	BOOST_CHECK_EQUAL(test_switch->out()(), 1);

	switch_param = 1; //change switch to second port
	BOOST_CHECK_EQUAL(test_switch->out()(), 2);
}

BOOST_AUTO_TEST_CASE(test_n_ary_switch_events)
{
	tests::owning_node root;
	event_source<int> source_1(&root.node());
	event_source<int> source_2(&root.node());
	auto test_switch = root.make_child_named<n_ary_switch<int, event_tag>>("switch");
	size_t switch_param = 0;
	state_source<size_t> config(&root.node(), [&switch_param](){ return switch_param; });
	event_sink_queue<int> buffer(&root.node());

	static_assert(!is_active_sink   <event_source<int>>{}, "");
	static_assert( is_active_source <event_source<int>>{}, "");
	static_assert(!is_passive_sink  <event_source<int>>{}, "");
	static_assert(!is_passive_source<event_source<int>>{}, "");

	source_1 >> test_switch->in(0);
	source_2 >> test_switch->in(1);
	config >> test_switch->control();
	test_switch->out() >> buffer;

	source_2.fire(2); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(buffer.empty(), true);
	source_1.fire(1); // tick forwarded source
	BOOST_CHECK_EQUAL(buffer.get(), 1);
	BOOST_CHECK_EQUAL(buffer.empty(), true);

	switch_param = 1;

	source_1.fire(1); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(buffer.empty(), true);
	source_2.fire(2); // tick forwarded source
	BOOST_CHECK_EQUAL(buffer.get(), 2);
	BOOST_CHECK_EQUAL(buffer.empty(), true);
}

BOOST_AUTO_TEST_CASE(watch_node)
{
	tests::owning_node root;

	int test_value = 0;

	auto watcher = watch([](int i){ return i < 0;}, int());

	int test_state = 1;
	state_source<int> source(&root.node(), [&test_state](){ return test_state; });
	event_sink<int> sink(&root.node(), [&test_value](int i){test_value = i;});

	source >> watcher.in();
	watcher.out() >> sink;

	watcher.check_tick()();

	BOOST_CHECK_EQUAL(test_value, 0);
	test_state = -1;

	watcher.check_tick()();
	BOOST_CHECK_EQUAL(test_value, -1);


}

BOOST_AUTO_TEST_CASE(test_on_changed)
{
	int test_value = 0;

	auto changed = on_changed<int>();

	int test_state = 1;
	pure::state_source<int> source([&test_state](){ return test_state; });
	pure::event_sink<int> sink([&test_value](int i){test_value = i;});

	source >> changed.in();
	changed.out() >> sink;

	changed.check_tick()();

	BOOST_CHECK_EQUAL(test_value, 1);

	test_state = 0;
	changed.check_tick()();
	BOOST_CHECK_EQUAL(test_value, 0);

	test_state = 1;
	changed.check_tick()();
	BOOST_CHECK_EQUAL(test_value, 1);

}
BOOST_AUTO_TEST_SUITE_END()
