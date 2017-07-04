#include "visualization.hpp"

#include <flexcore/scheduler/parallelregion.hpp>

#include <flexcore/extended/graph/graph.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <map>
#include <string>
#include <vector>
#include <ostream>

namespace fc
{

struct visualization::impl
{
	impl(const graph::connection_graph& g,
			const forest_t& f,
			const std::set<graph::graph_properties>& p)
		: graph_(g)
		, forest_(f)
		, ports_(p)
	{
	}

	static graph::graph_port_properties::port_type merge_property_types(
			const graph::graph_properties& source_node, const graph::graph_properties& sink_node);
	std::vector<graph::graph_properties> find_node_ports(
			graph::unique_id node_id) const;
	std::vector<graph::graph_properties> find_connectables(
			graph::unique_id node_id) const;
	std::string get_color(const parallel_region* region);
	void print_subgraph(typename forest_t::const_iterator node, std::ostream& stream);
	void print_ports(const std::vector<graph::graph_properties>& ports, unsigned long owner_hash,
			std::ostream& stream);
	static std::string escape_label(const std::string& label);

	const graph::connection_graph& graph_;
	const forest_t& forest_;
	const std::set<graph::graph_properties>& ports_;
	std::map<std::string, unsigned int> color_map_ {};
	unsigned int current_color_index_ = 0U;
};

graph::graph_port_properties::port_type visualization::impl::merge_property_types(
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

std::vector<graph::graph_properties> visualization::impl::find_node_ports(
		graph::unique_id node_id) const
{
	std::vector<graph::graph_properties> result;
	std::copy_if(std::begin(ports_), std::end(ports_), std::back_inserter(result),
			[&node_id](auto& port) { return port.port_properties.owning_node() == node_id; });
	return result;
}

std::vector<graph::graph_properties> visualization::impl::find_connectables(
		graph::unique_id node_id) const
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


void visualization::impl::print_subgraph(forest_t::const_iterator node, std::ostream& stream)
{
	const auto graph_info = (*node)->graph_info();
	const auto uuid = hash_value(graph_info.get_id());
	const auto& name = graph_info.name();
	stream << "subgraph cluster_" << uuid << " {\n";
	stream << "label=\"" << escape_label(name) << "\";\n";
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

std::string visualization::impl::get_color(const parallel_region* region)
{
	static constexpr auto no_region_color = "#ffffff";
	static constexpr auto out_of_color = "#000000";
	static constexpr auto colors = {"#809cda", "#e0eb5a", "#50bb3e", "#ed5690",
			"#64e487", "#ef5954", "#5ae5ac", "#ce76b5", "#48ab52", "#d8a4e7", "#76a73b", "#df8cb0",
			"#ceec77", "#58b1e1", "#e28f26", "#50d9e1", "#e27130", "#77e3c6", "#e2815e", "#42a68f",
			"#e7ba3b", "#de7c7f", "#a6e495", "#bc844e", "#5daa6e", "#c49739", "#dfeca1", "#999d31",
			"#e8b17c", "#9dbc70", "#dcd069", "#959551", "#d5c681", "#98ec6b", "#4a8bf0", "#98c234",
			"#9485dd", "#c2bf34", "#b875e7", "#e267cb"};

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
	assert(iter->second < colors.size());
	const auto color_iter = begin(colors) + iter->second;
	return *color_iter;
}

void visualization::impl::print_ports(const std::vector<graph::graph_properties>& ports,
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
			stream << escape_label(port.port_properties.description());
			stream << "\"];\n";
		}
	}
	else // extended ports
	{
		const auto printer = [&stream](auto&& port) {
			stream << "<" << hash_value(port.port_properties.id()) << ">";
			stream << escape_label(port.port_properties.description());
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

std::string visualization::impl::escape_label(const std::string& label)
{
	std::string s = boost::replace_all_copy(label, "<", "\\<");
	boost::replace_all(s, ">", "\\>");
	return s;
}

visualization::visualization(const graph::connection_graph& graph, const forest_t& forest)
	: pimpl{std::make_unique<impl>(graph, forest, graph.ports())}
{
	assert(pimpl);
}

void visualization::visualize(std::ostream& stream)
{
	pimpl->current_color_index_ = 0U;

	// nodes with their ports that are part of the forest
	stream << "digraph G {\n";
	stream << "rankdir=\"LR\"\n";
	pimpl->print_subgraph(pimpl->forest_.begin(), stream);

	// these are the ports wich are not part of the forest (ad hoc created)
	std::vector<graph::graph_properties> named_ports;
	std::copy_if(std::begin(pimpl->ports_), std::end(pimpl->ports_), std::back_inserter(named_ports),
			[this](auto&& graph_properties) { return graph_properties.node_properties.is_pure(); });
	for (auto& port : named_ports)
	{
		pimpl->print_ports({port}, hash_value(port.node_properties.get_id()), stream);
	}

	for (auto& edge : pimpl->graph_.edges())
	{
		const auto source_node = hash_value(edge.source.node_properties.get_id());
		const auto sink_node = hash_value(edge.sink.node_properties.get_id());
		const auto source_port = hash_value(edge.source.port_properties.id());
		const auto sink_port = hash_value(edge.sink.port_properties.id());
		using port_type = graph::graph_port_properties::port_type;

		stream << source_node << ":" << source_port << "->" << sink_node << ":" << sink_port;

		// draw arrow differently based on whether it is an event or state
		const auto merged_type = pimpl->merge_property_types(edge.source, edge.sink);
		if (merged_type == port_type::STATE)
		{
			stream << "[arrowhead=\"dot\"]";
		}
		stream << ";\n";
	}

	stream << "}\n";
}

visualization::~visualization() = default;

}
