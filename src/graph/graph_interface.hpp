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

static unique_id unique_id_count_hack = 0; //todo terrible terrible hack

/**
 * \brief Contains the information carried by a node of the dataflow graph
 */
class graph_node_properties
{
public:
	explicit graph_node_properties(const std::string name)
		: human_readable_name(name)
		, id(unique_id_count_hack++)
	{
	}

	std::string name() const { return human_readable_name; }
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
		: id(unique_id_count_hack++)
	{
	}
	unique_id get_id() const { return id; }
private:
	unique_id id;
};

}  // namespace graph
}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_INTERFACE_HPP_ */
