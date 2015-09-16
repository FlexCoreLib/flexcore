#ifndef SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_NODE_HPP_
#define SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_NODE_HPP_

// std
#include <memory>

namespace fc
{

/**
 * Stream source port that queries a node for the value
 * See test_stream_query_node.cpp for sample use inside a node
 *
 * Caller is responsible that the node passed to the constructor is not destroyed before the port
 */
template<
		class data_t, // token type
		class node_t, // type of node to be queried for the token
		data_t (node_t::*mem_fun_t)() // member function to call for the token
		>
class stream_query_node
{
public:
	stream_query_node(node_t* n) : node(n) {}

	data_t operator()()
	{
		return (node->*mem_fun_t)();
	}

private:
	node_t* node;
};

} // namespace fc

#endif /* SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_NODE_HPP_ */
