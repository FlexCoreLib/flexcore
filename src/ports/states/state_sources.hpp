#ifndef SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_
#define SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_

// std
#include <functional>
#include <memory>

#include "ports/detail/port_tags.hpp"

namespace fc
{

/**
 * State source port that queries a node for the value
 * See test_stream_query_node.cpp for sample use inside a node
 *
 * Caller is responsible that the node passed to the constructor is not destroyed before the port
 */
template<class data_t>
class state_source_call_function
{
public:
	/**
	 * \brief constructs state_source_function with function to call.
	 *
	 * \param f function which is called, when data is pulled from this source
	 * \pre f needs to be non empty function.
	 */
	explicit state_source_call_function(std::function<data_t()> f)
		: call(f)
	{
		assert(call);
	}

	data_t operator()() { return call(); }

private:
	std::function<data_t()> call;
};

/**
 * Simple StreamSource implementation that holds a state_source_with_setter.
 * Can be connected to any number of SinkConnections.
 * Is a SourceConnection.
 */
template<class data_t>
class state_source_with_setter
{
public:
	explicit state_source_with_setter(data_t d)
		: d(std::make_shared<data_t>(d))
	{}

	/// pull data
	data_t operator()() { return *d; }

	/// set current value
	void set(data_t d_) { *d = d_; }

private:
	std::shared_ptr<data_t> d;
};

/**
 * \brief minimal output port for states with templated operator()
 *
 * fulfills passive_source
 *
 * \tparam node_t type of owning node
 * \tparam tag_t is used as a template parameter for calling detail_out
 *         so the implementation of the node can distinguish between different ports.
 *         The value is not used.
 */
template<class node_t, class tag_t = detail::default_port_tag>
class state_source_tmpl
{
public:
	explicit state_source_tmpl(node_t& n)
		: node(n)
	{}

	/**
	 * \tparam token_t type of state expected
	 */
	template<class token_t>
	token_t operator()() { return node.template detail_out<tag_t, token_t>(); }

private:
	node_t& node;
};

// traits
template<class data_t> struct is_passive_source<state_source_with_setter<data_t>> : std::true_type {};
template<class data_t> struct is_passive_source<state_source_call_function<data_t>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_STREAM_SOURCES_STREAM_QUERY_FUNCTION_HPP_ */
