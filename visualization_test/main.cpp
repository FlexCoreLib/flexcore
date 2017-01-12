#include <fstream>

#include <flexcore/extended/base_node.hpp>
#include <flexcore/infrastructure.hpp>
#include <flexcore/ports.hpp>
#include <flexcore/pure/pure_ports.hpp>

using namespace fc;

int main()
{

	infrastructure infrastructure;

	auto first_region =
			infrastructure.add_region("first_region", thread::cycle_control::medium_tick);

	auto second_region =
			infrastructure.add_region("region two", thread::cycle_control::medium_tick);

	auto third_region = infrastructure.add_region("third two", thread::cycle_control::medium_tick);

	auto& root_node = infrastructure.node_owner();
	auto& node_a = root_node.make_child_named<owning_base_node>(first_region, "Node A");
	auto& node_g = root_node.make_child_named<owning_base_node>(second_region, "Node G");
	node_g.make_child_named<tree_base_node>(second_region, "Node U");

	event_sink<char> nodeA_port(&node_a, [](auto&&) {});

	state_source<std::string> root_port_1(&root_node, []() { return "test"; });
	event_sink<uint16_t> root_port_2(&root_node, [](auto&&) {});

	state_sink<std::string> nodeG_port1(&node_g);
	event_source<char> nodeG_port2(&node_g);

	auto pure_named = graph::named([](auto&& test) { return test; }, "My Pure functor");

	auto pure_named_source_2 = graph::named(pure::event_source<uint16_t>{}, "My Pure Port 2");

	root_port_1 >> nodeG_port1;
	nodeG_port2 >> [](auto&&) {};
	nodeG_port2 >> pure_named >> nodeA_port;
	pure_named_source_2 >> root_port_2;

	{
		std::ofstream out{"./out.dot"};
		infrastructure.visualize(out);
	}

	return 0;
}
