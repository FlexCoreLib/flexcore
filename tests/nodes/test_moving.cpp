#include "boost/test/unit_test.hpp"
#include "owning_node.hpp"
#include <flexcore/extended/base_node.hpp>

namespace
{
struct move_node : fc::tree_base_node
{
	static constexpr const char* default_name = "move_node";
	explicit move_node(const fc::node_args& args)
	    : tree_base_node(args)
	    , x(this, [] { return 3; })
		, y(this, [this](int i){ test_val = i;})
	{}

	void replace(state_source<int>&& other_s, event_sink<int>&& other_e)
	{
		x = std::move(other_s);
		y = std::move(other_e);
	}

	int test_val = 0;

	state_source<int> x;
	event_sink<int> y;
};
}

BOOST_AUTO_TEST_CASE(port_move_assignment)
{
	fc::tests::owning_node root("root");
	move_node& m = root.make_child<move_node>();

	fc::pure::state_sink<int> sink;
	fc::pure::event_source<int> source;

	fc::state_source<int> move_from_src(&m, [] { return 4; });
	fc::event_sink<int> move_from_sink(&m, [&m](int i){ m.test_val = i+1;});

	m.x >> sink;
	source >> m.y;
	m.replace(std::move(move_from_src), std::move(move_from_sink));
	BOOST_CHECK_EQUAL(sink.get(), 4);

	source.fire(99);
	//expect result from move_from_sink (in+1)
	BOOST_CHECK_EQUAL(m.test_val, 100);
}

