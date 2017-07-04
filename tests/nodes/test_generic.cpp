#include <boost/test/unit_test.hpp>

#include <flexcore/extended/nodes/generic.hpp>

#include "owning_node.hpp"
#include <pure/sink_fixture.hpp>

BOOST_AUTO_TEST_SUITE(test_generic_nodes)

using fc::operator>>;

BOOST_AUTO_TEST_CASE(test_n_ary_switch_state)
{
	fc::tests::owning_node root{};
	fc::state_source<int> one(&root.node(), [](){ return 1; });
	fc::state_source<int> two(&root.node(), [](){ return 2; });

	auto& test_switch = root.make_child_named<
			fc::n_ary_switch<int, fc::state_tag>>("switch");
	size_t switch_param{0};
	fc::state_source<size_t> config(
			&root.node(), [&switch_param](){ return switch_param; });

	one >> test_switch.in(0);
	two >> test_switch.in(1);
	config >> test_switch.control();

	BOOST_CHECK_EQUAL(test_switch.out()(), 1);

	switch_param = 1; //change switch to second port
	BOOST_CHECK_EQUAL(test_switch.out()(), 2);
}

BOOST_AUTO_TEST_CASE(test_n_ary_switch_events)
{
	fc::tests::owning_node root{};
	fc::event_source<int> source_1(&root.node());
	fc::event_source<int> source_2(&root.node());
	auto& test_switch = root.make_child_named<
			fc::n_ary_switch<int, fc::event_tag>>("switch");

	size_t switch_param{0};
	fc::state_source<size_t> config(
			&root.node(), [&switch_param](){ return switch_param; });

	std::vector<int> buffer;
	fc::event_sink<int> sink(
			&root.node(), [&buffer](auto in){buffer.push_back(in);});

	source_1 >> test_switch.in(0);
	source_2 >> test_switch.in(1);
	config >> test_switch.control();
	test_switch.out() >> sink;

	source_2.fire(2); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(buffer.empty(), true);
	source_1.fire(1); // tick forwarded source
	BOOST_CHECK_EQUAL(buffer.back(), 1);
	BOOST_CHECK_EQUAL(buffer.empty(), false);

	switch_param = 1;
	buffer.clear();

	source_1.fire(1); //tick source, currently not forwarded by switch
	BOOST_CHECK_EQUAL(buffer.empty(), true);
	source_2.fire(2); // tick forwarded source
	BOOST_CHECK_EQUAL(buffer.back(), 2);
	BOOST_CHECK_EQUAL(buffer.empty(), false);
}

BOOST_AUTO_TEST_CASE(watch_node)
{
	fc::tests::owning_node root{};

	int test_value{0};

	auto watcher = fc::watch([](int i){ return i < 0;}, int());

	int test_state{1};
	fc::state_source<int> source(&root.node(), [&test_state](){ return test_state; });
	fc::event_sink<int> sink(&root.node(), [&test_value](int i){test_value = i;});

	source >> watcher.in();
	watcher.out() >> sink;

	watcher.check_tick()();

	BOOST_CHECK_EQUAL(test_value, 0);
	BOOST_CHECK(test_value != test_state);
	test_state = -1;

	watcher.check_tick()();
	BOOST_CHECK_EQUAL(test_value, test_state);

}

BOOST_AUTO_TEST_CASE(test_on_changed)
{
	auto changed = fc::on_changed<int>();

	int test_state{1};
	fc::pure::state_source<int> source([&test_state](){ return test_state; });
	fc::pure::sink_fixture<int> sink{};

	source >> changed.in();
	changed.out() >> sink;

	changed.check_tick()();
	sink.expect(test_state);

	test_state = 0;
	changed.check_tick()();
	sink.expect(test_state);

	test_state = 1;
	changed.check_tick()();
	sink.expect(test_state);
}
BOOST_AUTO_TEST_SUITE_END()
