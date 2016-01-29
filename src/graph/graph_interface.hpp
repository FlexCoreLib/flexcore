#ifndef SRC_GRAPH_GRAPH_INTERFACE_HPP_
#define SRC_GRAPH_GRAPH_INTERFACE_HPP_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

/**
 * This file contains the classes which define the interface between
 * the flexcore connection logic with its connectables, ports and nodes on one side
 * and the tools to visualize and work on the dataflow graph.
 */

namespace fc
{
namespace graph
{

/**
 * \brief Contains the information carried by a node of the dataflow graph
 */
class graph_node_properties
{
public:
	explicit graph_node_properties(const std::string name)
		: human_readable_name(name)
		, id(boost::uuids::random_generator()())
	{
	}

	typedef boost::uuids::uuid unique_id;

	std::string name() const { return human_readable_name; }
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

}  // namespace graph
}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_INTERFACE_HPP_ */
