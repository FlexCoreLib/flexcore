#ifndef SRC_GRAPH_GRAPH_CONNECTABLE_HPP_
#define SRC_GRAPH_GRAPH_CONNECTABLE_HPP_

#include <graph/graph.hpp>
#include <core/connection.hpp>
#include <ports/connection_util.hpp>

#include <core/detail/connection_utils.hpp>
#include <ports/connection_util.hpp>

namespace fc
{
namespace graph
{

namespace detail
{
	struct graph_adder;
}

/**
 * \brief Mixin for connectables which adds additional information for abstract graph.
 *
 * Connections of graph_connectables will be added to the global abstract graph.
 *
 * \tparam base any connectable to wrap graph_connectable around.
 */
template<class base_t>
struct graph_connectable : base_t
{
	template<class... base_t_args>
	graph_connectable(const graph_node_properties& graph_info, const base_t_args&... args)
		: base_t(args...)
		, graph_info(graph_info)
	{
	}

	template<class arg_t,
			class base_check = base_t, //forward base_t to template param of method
			class = std::enable_if_t<is_active<base_check>::value>>
	decltype(auto) connect(arg_t&& conn)
	{
		std::cout << "Zonk!\n";
		//traverse connection and build up graph
		std::vector<graph_node_properties> node_list;

		node_list.push_back(graph_info);
		::fc::detail::apply(detail::graph_adder{node_list}, conn);

		if (node_list.size() >= 2)
		{
			for(auto it = node_list.begin()+1; it != node_list.end(); ++it)
			{
				add_to_graph(*(it-1), *it);
			}
		}


		return base_t::connect(std::forward<arg_t>(conn));
	}

	graph_node_properties graph_info;
};


template<class base_t>
auto make_graph_connectable(const base_t& base,
		const graph_node_properties& graph_info)
{
	return graph_connectable<base_t>{graph_info, base};
}

///Creates a graph_connectable with a human readable name.
template<class base_t>
auto named(const base_t& con, const std::string& name)
{
	return graph_connectable<base_t>{graph_node_properties{name}, con};
}

namespace detail
{
struct graph_adder
{
	std::vector<graph_node_properties>& node_list;

	template<class T>
	void operator()(T& /*node*/)
	{
		//do nothing
	}

	template<class base_t>
	void operator()(graph_connectable<base_t>& node)
	{
		node_list.push_back(node.graph_info);
	}
};
} //namespace detail

}  // namespace graph

template<class T> struct is_active_sink<graph::graph_connectable<T>> : is_active_sink<T> {};
template<class T> struct is_active_source<graph::graph_connectable<T>> : is_active_source<T> {};
template<class T> struct is_passive_sink<graph::graph_connectable<T>> : is_passive_sink<T> {};
template<class T> struct is_passive_source<graph::graph_connectable<T>> : is_passive_source<T> {};

}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_CONNECTABLE_HPP_ */
