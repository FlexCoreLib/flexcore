#include "visualization.hpp"

#include "flexcore/extended/graph/graph.hpp"

namespace fc
{

static graph::graph_port_properties::port_type merge_types(
		const graph::graph_properties& source_node, const graph::graph_properties& sink_node)
{
	using port_type = graph::graph_port_properties::port_type;
	port_type result = source_node.port_properties.type();
	if (result == port_type::UNDEFINED)
	{
		auto sink_type = sink_node.port_properties.type();
		assert(sink_type != port_type::UNDEFINED);
		result = sink_type;
	}
	return result;
}

static const std::string no_region_color = "#ffffff";
static const std::array<std::string, 15> colors{
		{"#d7aee6", "#eee4a5", "#a4b9e8", "#efb98d", "#71cdeb", "#f6a39f", "#8adbd3", "#eda4c1",
				"#97d1aa", "#ddc1e8", "#b6c68f", "#e8b0ac", "#d0f0c0", "#c9af8b", "#e6cda6"}};

visualization::visualization(const graph::connection_graph& graph, const forest_t& forest)
	: graph_(graph), forest_(forest)
{
}

void visualization::Visualize(std::ostream& stream)
{
	currentColorIndex_ = 0U;
	ports_ = graph_.ports();

	// nodes with their ports that are part of the forest
	stream << "digraph G {\n";
	printSubgraph(forest_.begin(), stream);

	// these are the ports wich are not part of the forest (ad hoc created) with graph::named
	std::vector<graph::graph_properties> named_ports{std::begin(ports_), std::end(ports_)};
	printPorts(named_ports, 0U, stream);

	for (auto& edge : graph_.edges())
	{
		const auto source_node = hash_value(edge.source.node_properties.get_id());
		const auto sink_node = hash_value(edge.sink.node_properties.get_id());
		const auto source_port = hash_value(edge.source.port_properties.id());
		const auto sink_port = hash_value(edge.sink.port_properties.id());
		using port_type = graph::graph_port_properties::port_type;

		stream << source_node;

		if (!edge.source.port_properties.pure())
			stream << ":" << source_port;

		stream << "->" << sink_node;

		if (!edge.sink.port_properties.pure())
			stream << ":" << sink_port;

		// draw arrow differently based on whether it is an event or state
		const auto merged_type = merge_types(edge.source, edge.sink);
		if (merged_type == port_type::STATE)
		{
			stream << "[arrowhead=\"dot\"]";
		}

		stream << ";\n";
	}

	stream << "}\n";
}

void visualization::printSubgraph(forest_t::const_iterator node, std::ostream& stream)
{
	const auto graph_info = (*node)->graph_info();
	const auto uuid = hash_value(graph_info.get_id());
	const auto& name = graph_info.name();
	stream << "subgraph cluster_" << uuid << " {\n";
	stream << "label=\"" << name << "\";\n";
	stream << "style=\"filled, bold, rounded\";\n";
	stream << "fillcolor=\"" << getColor(graph_info.region()) << "\";\n";

	const auto ports = extractNodePorts(graph_info.get_id());
	if (ports.empty())
	{
		stream << uuid << "[shape=\"plaintext\", label=\"\", width=0, height=0];\n";
	}
	else
	{
		printPorts(ports, uuid, stream);
	}

	for (auto iter = adobe::child_begin(node); iter != adobe::child_end(node); ++iter)
	{
		printSubgraph(iter.base(), stream);
	}
	stream << "}\n";
}

const std::string& visualization::getColor(const parallel_region* region)
{
	if (region == nullptr)
	{
		return no_region_color;
	}

	const auto key = region->get_id().key;
	auto iter = colorMap_.find(key);
	if (iter == std::end(colorMap_))
	{
		iter = colorMap_.emplace(key, currentColorIndex_).first;
		assert(currentColorIndex_ < colors.size());
		++currentColorIndex_;
	}
	return colors.at(iter->second);
}

std::vector<graph::graph_properties> visualization::extractNodePorts(
		graph::graph_port_properties::unique_id nodeID)
{
	std::vector<graph::graph_properties> result;
	std::copy_if(std::begin(ports_), std::end(ports_), std::back_inserter(result),
			[&nodeID](auto& port) { return port.port_properties.owning_node() == nodeID; });
	std::for_each(std::begin(result), std::end(result), [this](auto& port) { ports_.erase(port); });
	return result;
}

void visualization::printPorts(const std::vector<graph::graph_properties>& ports,
		unsigned long owner_hash, std::ostream& stream)
{
	if (ports.empty())
	{
		return;
	}

	// multiple ports per node: "default case"
	if (owner_hash != 0U)
	{
		stream << owner_hash << "[shape=\"record\", label=\"";
		bool first = true;
		for (auto& port : ports)
		{
			if (first)
				first = false;
			else
				stream << "|";
			stream << "<" << hash_value(port.port_properties.id()) << ">"
				   << port.port_properties.description();
		}
		stream << "\"]\n";
	}
	else
	{
		// named ports with pseudo node
		for (auto& port : ports)
		{
			stream << hash_value(port.node_properties.get_id());

			if (port.port_properties.pure())
			{
				stream << "[shape=\"plaintext\", label=\"\", width=0, height=0, fixedsize=true];\n";
				continue;
			}

			stream << "[shape=\"record\", style=\"dashed\", label=\"";
			stream << "<" << hash_value(port.port_properties.id()) << ">";
			stream << port.port_properties.description();
			stream << "\"];\n";
		}
	}
}
}
