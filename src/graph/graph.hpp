#ifndef SRC_GRAPH_GRAPH_HPP_
#define SRC_GRAPH_GRAPH_HPP_

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <map>

namespace fc
{
/**
 * \brief Contains all classes and functions to access
 * and read the abstract connection graph of a flexcore application.
 */
namespace graph
{

/**
 * \brief Contains the information carried by a node of the dataflow graph
 */
class graph_node_properties
{
public:
	typedef boost::uuids::uuid unique_id;

	explicit graph_node_properties(const std::string& name,
			unique_id id = boost::uuids::random_generator()())
		: human_readable_name(name)
		, id(id)
	{
	}


	const std::string& name() const { return human_readable_name; }
	std::string& name() { return human_readable_name; }
	unique_id get_id() const { return id; }
private:
	std::string human_readable_name;
	unique_id id;
};

/**
 * \brief Contains the information carried by ports
 * which are connected in the dataflow graph.
 */
class graph_port_information
{
public:
	graph_port_information()
		: id(boost::uuids::random_generator()())
	{
	}

	typedef boost::uuids::uuid unique_id;

	unique_id get_id() const { return id; }
private:
	unique_id id;
};

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

typedef boost::adjacency_list<boost::vecS,          // Store out-edges of each vertex in a std::list
							  boost::vecS,          // Store vertex set in a std::list
							  boost::directedS, // The dataflow graph is directed
							  vertex,                // vertex properties
							  edge                   // edge properties
							  > dataflow_graph_t;

/**
 * \brief The abstract connection graph of a flexcore application.
 *
 * Contains all nodes which where declared with the additional information
 * and edges between these nodes.
 * Currently implemented as a global singleton.
 *
 * \Todo Access to this class is currently not threadsafe
 * As mutable access is not guarded by mutexes.
 *
 * \invariant Number of vertices/nodes in dataflow_graph == vertex_map.size().
 */
class connection_graph
{
public:
	/// Static access to the singleton.
	static connection_graph& access()
	{
		static connection_graph s;
		return s;
	}

	connection_graph(const connection_graph&) = delete;

	/// Adds a new Connection containing two nodes and their ports to the graph.
	void add_connection(const graph_node_properties& source_node,
			const graph_port_information& source_port,
			const graph_node_properties& sink_node,
			const graph_port_information& sink_port)
	{
		//ToDo guard with mutex

		//check if vertex is already included, as add_vertex would add it again.
		if (vertex_map.find(source_node.get_id()) == vertex_map.end())
			vertex_map.emplace(
					source_node.get_id(),
					boost::add_vertex(vertex{source_node.name()}, dataflow_graph));
		if (vertex_map.find(sink_node.get_id()) == vertex_map.end())
			vertex_map.emplace(
					sink_node.get_id(),
					boost::add_vertex(vertex{sink_node.name()}, dataflow_graph));
		boost::add_edge(
				vertex_map[source_node.get_id()],
				vertex_map[sink_node.get_id()],
				graph::edge{"port: "
						+ std::to_string(
								boost::uuids::hash_value(source_port.get_id()))
						+ " to port: "
						+ std::to_string(
								boost::uuids::hash_value(sink_port.get_id()))},
				dataflow_graph);
	}

	/// Adds a new Connection without ports to the graph.
	void add_connection(const graph_node_properties& source_node,
			const graph_node_properties& sink_node)
	{
		//ToDo guard with mutex

		//check if vertex is already included, as add_vertex would add it again.
		if (vertex_map.find(source_node.get_id()) == vertex_map.end())
			vertex_map.emplace(
					source_node.get_id(),
					boost::add_vertex(vertex{source_node.name()}, dataflow_graph));
		if (vertex_map.find(sink_node.get_id()) == vertex_map.end())
			vertex_map.emplace(
					sink_node.get_id(),
					boost::add_vertex(vertex{sink_node.name()}, dataflow_graph));
		boost::add_edge(
				vertex_map[source_node.get_id()],
				vertex_map[sink_node.get_id()],
				edge{""},
				dataflow_graph);
	}

	dataflow_graph_t& get_boost_graph() { return dataflow_graph; }

private:
	connection_graph() = default;

	dataflow_graph_t dataflow_graph;
	std::map<graph_node_properties::unique_id,
			dataflow_graph_t::vertex_descriptor> vertex_map;

};

/**
 * \brief Adds a new edge to the graph representation.
 *
 * The edge is leading from source to sink through the two ports.
 */
void add_to_graph(const graph_node_properties& source_node,
		const graph_port_information& source_port,
		const graph_node_properties& sink_node,
		const graph_port_information& sink_port);

/**
 * \brief Adds a new edge to the graph representation.
 *
 * The edge is leading from source to sink without ports.
 */
void add_to_graph(const graph_node_properties& source_node,
		const graph_node_properties& sink_node);

/// Prints current state of the abstract graph in graphviz format to stream.
void print(std::ostream& stream);

}  // namespace graph
}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_HPP_ */
