#include <flexcore/extended/graph/graph.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <mutex>

namespace fc
{
namespace graph
{

/// Class containing the information of a node/vertex in the boost graph.
struct vertex
{
	std::string name;
	std::size_t uuid;
	std::size_t region;
};

/// Class containing the information of a connection/edge in the boost graph.
struct edge
{
	std::string name;
};

typedef boost::adjacency_list<boost::vecS, // Store out-edges of each vertex in a std::list
		boost::vecS,					   // Store vertex set in a std::list
		boost::directedS,				   // The dataflow graph is directed
		vertex,							   // vertex properties
		edge							   // edge properties
		>
		dataflow_graph_t;

struct connection_graph::impl
{
	/// Adds a new Connection without ports to the graph.
	void add_connection(const graph_properties& source_node, const graph_properties& sink_node);

	void add_port(const graph_properties& port_info);

	const std::set<graph_properties>& ports() const;
	const std::unordered_set<graph_edge>& edges() const;

	dataflow_graph_t dataflow_graph;
	std::map<graph_node_properties::unique_id, dataflow_graph_t::vertex_descriptor> vertex_map;
	std::unordered_set<graph_edge> edge_set;
	std::set<graph_properties> port_set;

	mutable std::mutex graph_mutex;
};

struct vertex_printer
{
	const dataflow_graph_t& graph;
	template <class Vertex>
	void operator()(std::ostream& out, const Vertex& v) const
	{
		auto Name = boost::get(&vertex::name, graph);
		auto Uuid = boost::get(&vertex::uuid, graph);
		auto Region = boost::get(&vertex::region, graph);
		out << std::hex;
		out << "[label=\"" << Name[v] << "\", uuid=\"0x" << Uuid[v] << "\", region=\"0x"
			<< Region[v] << "\"]";
		out << std::dec;
	}
};

connection_graph::connection_graph() : pimpl(std::make_unique<impl>())
{
}

graph_node_properties::graph_node_properties(
		const std::string& name, parallel_region* region, unique_id id, bool is_pure)
	: human_readable_name_(name), id_(id), region_(region), is_pure_(is_pure)
{
}

graph_node_properties::graph_node_properties(
		const std::string& name, parallel_region* region, bool is_pure)
	: graph_node_properties(name, region, boost::uuids::random_generator()(), is_pure)
{
}

graph_port_properties::graph_port_properties(
		std::string description, unique_id owning_node, port_type type)
	: description_(std::move(description))
	, owning_node_(std::move(owning_node))
	, id_(boost::uuids::random_generator()())
	, type_(std::move(type))
{
}

bool graph_port_properties::operator<(const graph_port_properties& o) const
{
	return id_ < o.id_;
}

connection_graph::~connection_graph() = default;

void connection_graph::print(std::ostream& stream)
{
	std::lock_guard<std::mutex> lock(pimpl->graph_mutex);
	const auto& graph = pimpl->dataflow_graph;
	boost::write_graphviz(stream, graph, vertex_printer{graph},
			boost::make_label_writer(boost::get(&edge::name, graph)));
}

void connection_graph::impl::add_connection(
		const graph_properties& source_node, const graph_properties& sink_node)
{
	std::lock_guard<std::mutex> lock(graph_mutex);
	auto region_to_hash = [](parallel_region* reg) {
		if (!reg)
			return ~std::size_t(0);

		return std::hash<std::string>{}(reg->get_id().key);
	};

	edge_set.emplace(source_node, sink_node);

	// check if vertex is already included, as add_vertex would add it again.
	if (vertex_map.find(source_node.node_properties.get_id()) == vertex_map.end())
		vertex_map.emplace(source_node.node_properties.get_id(),
				boost::add_vertex(vertex{source_node.node_properties.name(),
										  hash_value(source_node.node_properties.get_id()),
										  region_to_hash(source_node.node_properties.region())},
								   dataflow_graph));

	if (vertex_map.find(sink_node.node_properties.get_id()) == vertex_map.end())
		vertex_map.emplace(sink_node.node_properties.get_id(),
				boost::add_vertex(vertex{sink_node.node_properties.name(),
										  hash_value(sink_node.node_properties.get_id()),
										  region_to_hash(sink_node.node_properties.region())},
								   dataflow_graph));

	boost::add_edge(vertex_map[source_node.node_properties.get_id()],
			vertex_map[sink_node.node_properties.get_id()], edge{""}, dataflow_graph);
}

void connection_graph::impl::add_port(const graph_properties& port_info)
{
	std::lock_guard<std::mutex> lock(graph_mutex);
	port_set.emplace(port_info);
}

const std::set<graph_properties>& connection_graph::impl::ports() const
{
	std::lock_guard<std::mutex> lock(graph_mutex);
	return port_set;
}

const std::unordered_set<graph_edge>& connection_graph::impl::edges() const
{
	std::lock_guard<std::mutex> lock(graph_mutex);
	return edge_set;
}

void connection_graph::add_connection(
		const graph_properties& source_node, const graph_properties& sink_node)
{
	pimpl->add_connection(source_node, sink_node);
}

void connection_graph::add_port(const graph_properties& port_info)
{
	pimpl->add_port(port_info);
}

const std::set<graph_properties>& connection_graph::ports() const
{
	return pimpl->ports();
}

const std::unordered_set<graph_edge>& connection_graph::edges() const
{
	return pimpl->edges();
}

void connection_graph::clear_graph()
{
	std::lock_guard<std::mutex> lock(pimpl->graph_mutex);
	auto& graph = pimpl->dataflow_graph;
	graph.clear();
}

} // namespace graph
} // namespace fc
