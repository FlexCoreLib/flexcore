#include "graph/graph.hpp"

namespace fc
{
namespace graph
{

void add_to_graph(const graph_node_properties& source_node,
		const graph_port_information& source_port,
		const graph_node_properties& sink_node,
		const graph_port_information& sink_port)
{
	connection_graph::access().add_connection(
			source_node,
			source_port,
			sink_node,
			sink_port);
}

void add_to_graph(const graph_node_properties& source_node,
		const graph_node_properties& sink_node)
{
	connection_graph::access().add_connection(
			source_node,
			sink_node);
}

void print()
{
	const auto graph = connection_graph::access().get_boost_graph();
	boost::write_graphviz(std::cout, graph,
	     boost::make_label_writer(boost::get(&vertex::name, graph)),
	     boost::make_label_writer(boost::get(&edge::name, graph)));
}

} // namespace graph
} // namespace fc

