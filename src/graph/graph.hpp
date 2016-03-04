#ifndef SRC_GRAPH_GRAPH_HPP_
#define SRC_GRAPH_GRAPH_HPP_

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

	struct impl;
	friend void print(std::ostream& stream);

	connection_graph(const connection_graph&) = delete;

	/// Adds a new Connection containing two nodes and their ports to the graph.
	void add_connection(const graph_node_properties& source_node,
			const graph_port_information& source_port,
			const graph_node_properties& sink_node,
			const graph_port_information& sink_port);

	/// Adds a new Connection without ports to the graph.
	void add_connection(const graph_node_properties& source_node,
			const graph_node_properties& sink_node);

	~connection_graph();

private:
	connection_graph();

	std::unique_ptr<impl> pimpl;
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
