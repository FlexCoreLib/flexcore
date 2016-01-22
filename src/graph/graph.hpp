#ifndef SRC_GRAPH_GRAPH_HPP_
#define SRC_GRAPH_GRAPH_HPP_

#include <graph/graph_interface.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <map>

namespace fc
{
namespace graph
{

/// Class containing the information of a node/vertex in the boost graph.
struct vertex
{
	std::string name;
};

/// Class containing the information of a connection/edge in the boost graph.
struct edge
{
	std::string name;
};

typedef boost::adjacency_list<boost::listS,          // Store out-edges of each vertex in a std::list
                              boost::listS,          // Store vertex set in a std::list
                              boost::directedS, // The dataflow graph is directed
                              vertex,                // vertex properties
                              edge                   // edge properties
                              > dataflow_graph_t;


class connection_graph
{
public:
	static connection_graph& access()
	{
	     static connection_graph s;
	     return s;
	}

	connection_graph(const connection_graph&) = delete;

	void add_connection(const graph_node_properties& source_node,
			const graph_port_information& source_port,
			const graph_node_properties& sink_node,
			const graph_port_information& sink_port)
	{
		//ToDo guard with mutex
		vertex_map.emplace(
				source_node.get_id(),
				boost::add_vertex(vertex{source_node.name()}, dataflow_graph));
		vertex_map.emplace(
				sink_node.get_id(),
				boost::add_vertex(vertex{sink_node.name()}, dataflow_graph));
		boost::add_edge(
				vertex_map[source_node.get_id()],
				vertex_map[sink_node.get_id()],
				edge{std::to_string(source_port.get_id())
						+ " ----> "
						+ std::to_string(sink_port.get_id())},
				dataflow_graph);
	}

	auto& get_boost_graph() { return dataflow_graph; }

private:
	connection_graph()
	{
	}

	dataflow_graph_t dataflow_graph;
	std::map<unique_id, dataflow_graph_t::vertex_descriptor> vertex_map;

};

/**
 * \brief Adds a new edge to the graph representation.
 *
 * The edge is leading from source to sink through the two ports.
 */
inline void ad_to_graph(const graph_node_properties& source_node,
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

inline void print()
{
	auto& graph = connection_graph::access().get_boost_graph();

	for (auto e = boost::edges(graph).first; e != boost::edges(graph).second; ++e)
	{
		std::cout << graph[ boost::source(*e, graph)].name << " -> ";
				std::cout << graph[*e].name << " -> ";
				std::cout << graph[boost::target(*e, graph)].name << "\n";
	}
}

}  // namespace graph
}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_HPP_ */
