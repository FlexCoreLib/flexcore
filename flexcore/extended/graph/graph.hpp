#ifndef SRC_GRAPH_GRAPH_HPP_
#define SRC_GRAPH_GRAPH_HPP_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <flexcore/scheduler/parallelregion.hpp>

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

	explicit graph_node_properties(const std::string& name, parallel_region* region,
	                               unique_id id = boost::uuids::random_generator()())
	    : human_readable_name(name), id(id), region_(region)
	{
	}
	explicit graph_node_properties(const std::string& name) : graph_node_properties(name, nullptr)
	{
	}

	const std::string& name() const { return human_readable_name; }
	std::string& name() { return human_readable_name; }
	unique_id get_id() const { return id; }
	parallel_region* region() const { return region_; }
private:
	std::string human_readable_name;
	unique_id id;
	parallel_region* region_;
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
	connection_graph();
	connection_graph(const connection_graph&) = delete;

	/// Adds a new Connection without ports to the graph.
	void add_connection(const graph_node_properties& source_node,
	                    const graph_node_properties& sink_node);

	/// Prints current state of the abstract graph in graphviz format to stream.
	void print(std::ostream& stream);

	/// deleted the current graph
	void clear_graph();

	~connection_graph();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

}  // namespace graph
}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_HPP_ */
