#include <flexcore/extended/graph/graph.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

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
};

/// Class containing the information of a connection/edge in the boost graph.
struct edge
{
	std::string name;
};

typedef boost::adjacency_list<boost::vecS,          // Store out-edges of each vertex in a std::list
							  boost::vecS,          // Store vertex set in a std::list
							  boost::directedS, // The dataflow graph is directed
							  vertex,                // vertex properties
							  edge                   // edge properties
							  > dataflow_graph_t;

struct connection_graph::impl
{
	/// Adds a new Connection without ports to the graph.
	void add_connection(const graph_node_properties& source_node,
			const graph_node_properties& sink_node);

	dataflow_graph_t dataflow_graph;
	std::map<graph_node_properties::unique_id,
			dataflow_graph_t::vertex_descriptor> vertex_map;

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
		out << "[label=\"" << Name[v] << "\", uuid=\"" << std::hex << Uuid[v] << "\"]";
	}
};

connection_graph::connection_graph()
	: pimpl(std::make_unique<impl>())
{
}

connection_graph::~connection_graph() = default;

void connection_graph::print(std::ostream& stream)
{
	std::lock_guard<std::mutex> lock(pimpl->graph_mutex);
	const auto& graph = pimpl->dataflow_graph;
	boost::write_graphviz(stream, graph,
		vertex_printer{graph},
		boost::make_label_writer(boost::get(&edge::name, graph)));
}

void connection_graph::impl::add_connection(const graph_node_properties& source_node,
		const graph_node_properties& sink_node)
{
	std::lock_guard<std::mutex> lock(graph_mutex);

	//check if vertex is already included, as add_vertex would add it again.
	if (vertex_map.find(source_node.get_id()) == vertex_map.end())
		vertex_map.emplace(source_node.get_id(), boost::add_vertex(vertex {
				source_node.name(), hash_value(source_node.get_id())}, dataflow_graph));

	if (vertex_map.find(sink_node.get_id()) == vertex_map.end())
		vertex_map.emplace(sink_node.get_id(), boost::add_vertex(vertex {
				sink_node.name(), hash_value(sink_node.get_id())}, dataflow_graph));

	boost::add_edge(vertex_map[source_node.get_id()],
			vertex_map[sink_node.get_id()], edge { "" }, dataflow_graph);
}

void connection_graph::add_connection(const graph_node_properties& source_node,
		const graph_node_properties& sink_node)
{
	pimpl->add_connection(source_node, sink_node);
}

void connection_graph::clear_graph()
{
	std::lock_guard<std::mutex> lock(pimpl->graph_mutex);
	auto& graph = pimpl->dataflow_graph;
	graph.clear();
}

} // namespace graph
} // namespace fc

