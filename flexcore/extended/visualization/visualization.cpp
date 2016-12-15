#include "visualization.hpp"

#include "flexcore/extended/graph/graph.hpp"

namespace fc
{

void visualization::Visualize(std::ostream& stream, const graph::connection_graph& graph,
							  const forest_t& forest) const
{
	stream << "digraph G {\n";
	printSubgraph(forest.begin(), stream, graph);
	stream << "}\n";
}

void visualization::printSubgraph(forest_t::const_iterator node,
								  std::ostream& stream,
								  const graph::connection_graph& graph) const
{
	auto uuid = hash_value((*node)->graph_info().get_id());
	auto& name = (*node)->graph_info().name();
	stream << "subgraph cluster_" << uuid << " {\n";
	stream << "label=\"" << name << "\";\n";
	stream << "style=\"filled, bold, rounded\";\n";
	// TODO: generate color from region
	// TODO: print ports here

	stream << uuid << "[shape=\"plaintext\", label=\"\", width=0, height=0];\n";

	for (auto iter =  adobe::child_begin(node); iter != adobe::child_end(node); ++iter)
	{
		printSubgraph(iter.base(), stream, graph);
	}
	stream << "}\n";
}

}
