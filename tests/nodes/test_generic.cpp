#include <boost/test/unit_test.hpp>

#include <flexcore/extended/nodes/generic.hpp>
#include <pure/events/event_sink_with_queue.hpp>

#include "owning_node.hpp"

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_generic_nodes)

BOOST_AUTO_TEST_CASE(test_n_ary_switch_state)
{
	tests::owning_node root;
	state_source<int> one(&root.node(), [](){ return 1; });
	state_source<int> two(&root.node(), [](){ return 2; });

	auto& test_switch = root.make_child_named<n_ary_switch<int, state_tag>>("switch");
	size_t switch_param = 0;
	state_source<size_t> config(&root.node(), [&switch_param](){ return switch_param; });

	one >> test_switch.in(0);
	two >> test_switch.in(1);
	config >> test_switch.control();

	BOOST_CHECK_EQUAL(test_switch.out()(), 1);

	switch_param = 1; //change switch to second port
	BOOST_CHECK_EQUAL(test_switch.out()(), 2);
}

BOOST_AUTO_TEST_CASE(test_n_ary_switch_events)
{
	tests::owning_node root;
	event_source<int> source_1(&root.node());
	event_source<int> source_2(&root.node());
	auto& test_switch = root.make_child_named<n_ary_switch<int, event_tag>>("switch");
	size_t switch_param = 0;
	state_source<size_t> config(&root.node(), [&switch_param](){ return switch_param; });

	std::vector<int> result_buffer;
	event_sink<int> buffer(&root.node(), [&result_buffer](auto in){result_buffer.push_back(in);});

	static_assert(!is_active_sink   <event_source<int>>{}, "");
	static_assert( is_active_source <event_source<int>>{}, "");
	static_assert(!is_passive_sink  <event_source<int>>{}, "");
	static_assert(!is_passive_source<event_source<int>>{}, "");

	source_1 >> test_switch.in(0);
	source_2 >> test_switch.in(1);
	config >> test_switch.control();
	test_switch.out() >> buffer;

	source_2.fire(2); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(result_buffer.empty(), true);
	source_1.fire(1); // tick forwarded source
	BOOST_CHECK_EQUAL(result_buffer.back(), 1);
	BOOST_CHECK_EQUAL(result_buffer.empty(), false);

	switch_param = 1;
	result_buffer.clear();

	source_1.fire(1); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(result_buffer.empty(), true);
	source_2.fire(2); // tick forwarded source
	BOOST_CHECK_EQUAL(result_buffer.back(), 2);
	BOOST_CHECK_EQUAL(result_buffer.empty(), false);
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
