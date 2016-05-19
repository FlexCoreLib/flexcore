#include <boost/test/unit_test.hpp>
#include <flexcore/extended/nodes/terminal.hpp>
#include <flexcore/ports.hpp>
#include <flexcore/pure/pure_node.hpp>


#include <nodes/owning_node.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_terminal_node)

BOOST_AUTO_TEST_CASE(test_event_terminal)
{
	const float test_val = 1.234;

	{
	float test_out = 0.0;

	event_terminal<float, pure::pure_node> pure_terminal;

	pure::event_source<float> pure_source;
	pure::event_sink<float> pure_sink{[&test_out](float in)
			{test_out = in;}};

	pure_source >> pure_terminal.in();
	pure_terminal.out() >> pure_sink;

	pure_source.fire(test_val);
	BOOST_CHECK_EQUAL(test_val, test_out);
	}

	{
	float test_out = 0.0;
	tests::owning_node root;
	auto& tree_terminal = root.make_child_named<event_terminal<float>>("terminal");

	event_source<float> tree_source{&root.node()};
	event_sink<float> tree_sink{&root.node(), [&test_out](float in)
		{test_out = in;}};

	tree_source >> tree_terminal.in();
	tree_terminal.out() >> tree_sink;

	tree_source.fire(test_val);
	BOOST_CHECK_EQUAL(test_val, test_out);
	}
}

BOOST_AUTO_TEST_CASE(test_state_terminal)
{
	const float test_val = 1.234;

	state_terminal<float, pure::pure_node> pure_terminal;

	pure::state_source<float> pure_source{[test_val](){ return test_val;}};
	pure::state_sink<float> pure_sink;

	pure_source >> pure_terminal.in();
	pure_terminal.out() >> pure_sink;

	BOOST_CHECK_EQUAL(test_val, pure_sink.get());

	tests::owning_node root;
	auto& tree_terminal = root.make_child_named<state_terminal<float>>("terminal");

	state_source<float> tree_source{&root.node(),[test_val](){ return test_val;}};
	state_sink<float> tree_sink{&root.node()};

	tree_source >> tree_terminal.in();
	tree_terminal.out() >> tree_sink;

	BOOST_CHECK_EQUAL(test_val, tree_sink.get());

}

BOOST_AUTO_TEST_SUITE_END()
