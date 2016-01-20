#ifndef SRC_GRAPH_GRAPH_INTERFACE_HPP_
#define SRC_GRAPH_GRAPH_INTERFACE_HPP_

/**
 * This file contains the classes which define the interface between
 * the flexcore connection logic with its connectables, ports and nodes on one side
 * and the tools to visualize and work on the dataflow graph.
 */

namespace fc
{
namespace graph
{

typedef int unique_id; //todo

/**
 * \brief Contains the information carried by a node of the dataflow graph
 */
class graph_node_properties
{
public:
	explicit graph_node_properties(const std::string name)
		: human_readable_name(name)
	{
	}

	std::string name() const { return human_readable_name; }
private:
	std::string human_readable_name;
	unique_id id = 0;
};

/**
 * \brief Contains the information carried by ports
 * which are connected in the dataflow graph.
 */
class graph_port_information
{
public:

//private:
	unique_id id = 0;
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
	//ToDo guard with mutex and fill datastructure
	std::cout << "Edge from: " << source_node.name() << "." << source_port.id;
	std::cout << " to: " << sink_node.name() << "." << sink_port.id << "\n";
}

}  // namespace graph
}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_INTERFACE_HPP_ */
