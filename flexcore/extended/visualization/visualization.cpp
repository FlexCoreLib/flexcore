#include "visualization.hpp"

#include "flexcore/extended/graph/graph.hpp"

namespace fc
{

visualization::visualization(const graph::connection_graph& graph, const forest_t& forest)
	: graph_(graph), forest_(forest), ports_(graph.ports())
{
}
graph::graph_port_properties::port_type visualization::merge_property_types(
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

std::vector<graph::graph_properties> visualization::find_node_ports(
		graph::graph_port_properties::unique_id node_id) const
{
	std::vector<graph::graph_properties> result;
	std::copy_if(std::begin(ports_), std::end(ports_), std::back_inserter(result),
			[&node_id](auto& port) { return port.port_properties.owning_node() == node_id; });
	return result;
}

std::vector<graph::graph_properties> visualization::find_connectables(
		graph::graph_port_properties::unique_id node_id) const
{
	std::vector<graph::graph_properties> result;
	for (auto& edge : graph_.edges())
	{
		auto& source_props = edge.source.port_properties;
		if (source_props.id() == node_id && edge.sink.node_properties.is_pure())
		{
			result.push_back(edge.sink);
		}
	}
	return result;
}

void visualization::visualize(std::ostream& stream)
{
	current_color_index_ = 0U;

	// nodes with their ports that are part of the forest
	stream << "digraph G {\n";
	print_subgraph(forest_.begin(), stream);

	// these are the ports wich are not part of the forest (ad hoc created)
	std::vector<graph::graph_properties> named_ports;
	std::copy_if(std::begin(ports_), std::end(ports_), std::back_inserter(named_ports),
			[this](auto&& graph_properties) { return graph_properties.node_properties.is_pure(); });
	for (auto& port : named_ports)
	{
		print_ports({port}, hash_value(port.node_properties.get_id()), stream);
	}

	for (auto& edge : graph_.edges())
	{
		const auto source_node = hash_value(edge.source.node_properties.get_id());
		const auto sink_node = hash_value(edge.sink.node_properties.get_id());
		const auto source_port = hash_value(edge.source.port_properties.id());
		const auto sink_port = hash_value(edge.sink.port_properties.id());
		using port_type = graph::graph_port_properties::port_type;

		stream << source_node << ":" << source_port << "->" << sink_node << ":" << sink_port;

		// draw arrow differently based on whether it is an event or state
		const auto merged_type = merge_property_types(edge.source, edge.sink);
		if (merged_type == port_type::STATE)
		{
			stream << "[arrowhead=\"dot\"]";
		}
		stream << ";\n";
	}

	stream << "}\n";
}

void visualization::print_subgraph(forest_t::const_iterator node, std::ostream& stream)
{
	const auto graph_info = (*node)->graph_info();
	const auto uuid = hash_value(graph_info.get_id());
	const auto& name = graph_info.name();
	stream << "subgraph cluster_" << uuid << " {\n";
	stream << "label=\"" << name << "\";\n";
	stream << "style=\"filled, bold, rounded\";\n";
	stream << "fillcolor=\"" << get_color(graph_info.region()) << "\";\n";

	const auto ports = find_node_ports(graph_info.get_id());
	if (ports.empty())
	{
		stream << uuid << "[shape=\"plaintext\", label=\"\", width=0, height=0];\n";
	}
	else
	{
		print_ports(ports, uuid, stream);
	}

	for (auto iter = adobe::child_begin(node); iter != adobe::child_end(node); ++iter)
	{
		print_subgraph(iter.base(), stream);
	}
	stream << "}\n";

	for (auto& port : ports)
	{
		for (auto& connectable : find_connectables(port.port_properties.id()))
		{
			print_ports({connectable}, hash_value(connectable.node_properties.get_id()), stream);
		}
	}
}

const std::string& visualization::get_color(const parallel_region* region)
{
	static const std::string no_region_color = "#ffffff";
	static const std::string out_of_color = "#000000";
	static const std::array<std::string, 40> colors{{"#809cda", "#e0eb5a", "#b875e7", "#50bb3e",
			"#e267cb", "#ed5690", "#64e487", "#ef5954", "#5ae5ac", "#ce76b5", "#48ab52", "#d8a4e7",
			"#76a73b", "#df8cb0", "#ceec77", "#58b1e1", "#e28f26", "#50d9e1", "#e27130", "#77e3c6",
			"#e2815e", "#42a68f", "#e7ba3b", "#de7c7f", "#a6e495", "#bc844e", "#5daa6e", "#c49739",
			"#dfeca1", "#999d31", "#e8b17c", "#9dbc70", "#dcd069", "#959551", "#d5c681", "#98ec6b",
			"#4a8bf0", "#98c234", "#9485dd", "#c2bf34"}};

	if (region == nullptr)
	{
		return no_region_color;
	}

	const auto key = region->get_id().key;
	auto iter = color_map_.find(key);
	if (iter == std::end(color_map_))
	{
		if (current_color_index_ >= colors.size())
			return out_of_color;

		iter = color_map_.emplace(key, current_color_index_).first;
		++current_color_index_;
	}
	return colors.at(iter->second);
}

void visualization::print_ports(const std::vector<graph::graph_properties>& ports,
		unsigned long owner_hash, std::ostream& stream)
{
	if (ports.empty())
	{
		return;
	}

	// named ports with pseudo node
	if (ports.size() == 1 && ports.front().node_properties.is_pure())
	{
		for (auto& port : ports)
		{
			stream << hash_value(port.node_properties.get_id());
			stream << "[shape=\"record\", style=\"dashed, filled, bold\", fillcolor=\"white\" "
					  "label=\"";
			stream << "<" << hash_value(port.port_properties.id()) << ">";
			stream << port.port_properties.description();
			stream << "\"];\n";
		}
	}
	else // extended ports
	{
		const auto printer = [&stream](auto&& port) {
			stream << "<" << hash_value(port.port_properties.id()) << ">"
				   << port.port_properties.description();
		};

		stream << owner_hash << "[shape=\"record\", label=\"";
		printer(ports.front());
		for (auto iter = ++std::begin(ports); iter != std::end(ports); ++iter)
		{
			stream << "|";
			printer(*iter);
		}
		stream << "\"]\n";
	}
}
}
