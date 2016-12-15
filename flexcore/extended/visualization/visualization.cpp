#include "visualization.hpp"

#include "flexcore/extended/graph/graph.hpp"

namespace fc
{

static const std::array<std::string, 15> colors
{{"#d7aee6", "#eee4a5", "#a4b9e8", "#efb98d", "#71cdeb", "#f6a39f", "#8adbd3", "#eda4c1", "#97d1aa",
"#ddc1e8", "#b6c68f", "#e8b0ac", "#d0f0c0", "#c9af8b", "#e6cda6"}};

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
	auto graph_info = (*node)->graph_info();
	auto uuid = hash_value(graph_info.get_id());
	auto& name = graph_info.name();
	stream << "subgraph cluster_" << uuid << " {\n";
	stream << "label=\"" << name << "\";\n";
	stream << "style=\"filled, bold, rounded\";\n";
	stream << "fillcolor=\"" << getColor(*graph_info.region()) <<  "\";\n";
	// TODO: generate color from region
	// TODO: print ports here

	stream << uuid << "[shape=\"plaintext\", label=\"\", width=0, height=0];\n";

	for (auto iter =  adobe::child_begin(node); iter != adobe::child_end(node); ++iter)
	{
		printSubgraph(iter.base(), stream, graph);
	}
	stream << "}\n";
}

const std::string& visualization::getColor(const parallel_region& region) const
{
	auto key = region.get_id().key;
	auto iter = colorMap_.find(key);
	if (iter == std::end(colorMap_))
	{
		iter = colorMap_.emplace(key, currentColorIndex_).first;
		assert(currentColorIndex_ < colors.size());
		++currentColorIndex_;
	}
	return colors[iter->second];
}

}
