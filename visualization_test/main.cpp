#include <fstream>

#include <flexcore/extended/base_node.hpp>
#include <flexcore/ports.hpp>
#include <flexcore/infrastructure.hpp>
#include <flexcore/pure/pure_ports.hpp>

using namespace fc;

int main()
{

	infrastructure infrastructure;

	auto first_region = infrastructure.add_region(
			"first_region",
			thread::cycle_control::medium_tick);

	auto second_region = infrastructure.add_region(
			"region two",
			thread::cycle_control::medium_tick);

	auto third_region = infrastructure.add_region(
			"third two",
			thread::cycle_control::medium_tick);

	auto& root_node = infrastructure.node_owner();
	auto& node_a = root_node.make_child_named<owning_base_node>(first_region, "Node A");
	auto& node_g = root_node.make_child_named<owning_base_node>(second_region, "Node G");
	node_g.make_child_named<tree_base_node>(second_region, "Node U");

	state_sink<uint8_t> nodeA_port(&node_a);

	event_source<std::string> root_port_1(&root_node);
	event_sink<uint16_t> root_port_2(&root_node, [](auto&&) {});

	event_sink<std::string> nodeG_port1(&node_g, [](auto&&) {});
	event_sink<int> nodeG_port2(&node_g, [](auto&&) {});

	pure::state_source<uint8_t> pure_source_1{[]{ return 42; }};
	pure::event_source<uint16_t> pure_source_2;

	pure_source_1 >> nodeA_port;
	pure_source_2 >> root_port_2;

	root_port_1 >> nodeG_port1;

	{
		std::ofstream graph_out{"./graph.dot"};
		std::ofstream forest_out{"./forest.json"};
		infrastructure.print(graph_out, forest_out);
	}

	return 0;
}
