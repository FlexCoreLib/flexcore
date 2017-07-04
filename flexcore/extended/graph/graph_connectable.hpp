#ifndef SRC_GRAPH_GRAPH_CONNECTABLE_HPP_
#define SRC_GRAPH_GRAPH_CONNECTABLE_HPP_

#include <flexcore/core/connection.hpp>
#include <flexcore/core/connection_util.hpp>
#include <flexcore/core/detail/connection_utils.hpp>
#include <flexcore/extended/graph/graph.hpp>
#include <flexcore/extended/graph/traits.hpp>
#include <flexcore/utils/demangle.hpp>

#include <cassert>

namespace fc
{
namespace graph
{

class connection_graph;

template <class base_t>
struct graph_connectable;

namespace detail
{
/// functor to use in connection applyer which adds graph_unfo to list
struct graph_adder
{
	std::vector<graph_properties>& node_list;

	template <class T>
	void operator()(T& /*node*/, typename std::enable_if<!has_graph_info<T>(0)>::type* = nullptr)
	{
	}

	// non_const ref, because applyer might forward non_const
	template <class T>
	void operator()(T& node, typename std::enable_if<has_graph_info<T>(0)>::type* = nullptr)
	{
		node_list.emplace_back(node.graph_info, node.graph_port_info);
	}
};

template <class T>
auto port_description(const std::string& node_name)
		-> std::enable_if_t<has_token_type<T>(0), std::string>
{
	if (!node_name.empty())
		return "'" + node_name + "'";
	else
		return demangle(typeid(typename T::token_t).name());
}

/// \post !result.empty()
template <class T>
auto port_description(const std::string& node_name)
		-> std::enable_if_t<!has_token_type<T>(0), std::string>
{
	if (!node_name.empty())
		return "'" + node_name + "'";
	else
		return "'AdHoc'";
}

template <class T>
auto graph_object(const T& connectable) -> std::enable_if_t<has_graph_info<T>(0), connection_graph*>
{
	return connectable.graph;
}

template <class T>
auto graph_object(const T&) -> std::enable_if_t<!has_graph_info<T>(0), connection_graph*>
{
	return nullptr;
}

template <class T, typename std::enable_if_t<has_graph_info<T>(0), int> = 0>
void set_graph_object(T& connectable, connection_graph* graph)
{
	connectable.graph = graph;
}

template <class T, typename std::enable_if_t<!has_graph_info<T>(0), int> = 0>
void set_graph_object(T&, connection_graph*)
{
}

} // namespace detail

/**
 * \brief Mixin for connectables which adds additional information for abstract graph.
 *
 * Connections of graph_connectables will be added to the global abstract graph.
 *
 * \tparam base_t any connectable to wrap graph_connectable around.
 */
template <class base_t>
struct graph_connectable : base_t
{
	/**
	 * \brief Construct graph_connectable with access to graph::connection_graph
	 * \param graph access to central connection_graph
	 * \param graph_info information for the connection_graph of this object
	 * \param args arguments forwarded to base class
	 * \post graph != nullptr
	 */
	template <class... base_t_args>
	graph_connectable(graph::connection_graph& central_graph, const graph_node_properties& graph_info,
			base_t_args&&... args)
		: base_t(std::forward<base_t_args>(args)...)
		, graph_info(graph_info)
		, graph_port_info(detail::port_description<base_t>(std::string{}), graph_info.get_id(),
				  graph_port_properties::to_port_type<base_t>())
		, graph(&central_graph)
	{
		graph->add_port({graph_info, graph_port_info});
		assert(graph != nullptr);
	}

	///Construct graph_connectables without access to connection_graph
	template <class... base_t_args>
	graph_connectable(const graph_node_properties& graph_info, base_t_args&&... args)
		: base_t(std::forward<base_t_args>(args)...)
		, graph_info(graph_info)
		, graph_port_info(detail::port_description<base_t>(graph_info.name()), graph_info.get_id(),
				  graph_port_properties::to_port_type<base_t>())
		, graph(nullptr)
	{
	}

	///Adds connection to graph and then forwards call to base_t::connect
	template <class arg_t,
			class base_check = base_t, // forward base_t to template param of method
			class = std::enable_if_t<class_is_active_v<base_check>>>
	decltype(auto) connect(arg_t&& conn)
	{
		auto* current_graph = graph;

		if (!current_graph)
			current_graph = detail::graph_object(conn);

		if (!current_graph)
			return base_t::connect(std::forward<arg_t>(conn));

		assert(current_graph != nullptr);

		// hijack graph object
		detail::set_graph_object(conn, current_graph);
		graph = current_graph;
		assert(graph != nullptr);

		// traverse connection and build up graph
		if (is_active_sink<base_t>{}) // condition set at compile_time
		{
			add_state_connection(conn, *graph);
		}
		else if (is_active_source<base_t>{}) // condition set at compile_time
		{
			add_event_connection(conn, *graph);
		}

		return base_t::connect(std::forward<arg_t>(conn));
	}

	///graph_info needs to be public as it is checked by graph adding methods.
	graph_node_properties graph_info;
	graph_port_properties graph_port_info;
	graph::connection_graph* graph;
private:

	template <class connection_t>
	void add_state_connection(connection_t& conn, graph::connection_graph& current_graph) const
	{
		std::vector<graph_properties> node_list;

		::fc::detail::apply(detail::graph_adder{node_list}, conn);
		// state_sink is last, thus added after connection
		node_list.emplace_back(graph_info, graph_port_info);

		if (node_list.size() >= 2)
			for (auto it = node_list.begin() + 1; it != node_list.end(); ++it)
				current_graph.add_connection(*(it - 1), *it);

		for (auto& node : node_list)
			current_graph.add_port(node);
	}

	template <class connection_t>
	void add_event_connection(connection_t& conn, graph::connection_graph& current_graph) const
	{
		std::vector<graph_properties> node_list;

		// event source is first, thus added before connection
		node_list.emplace_back(graph_info, graph_port_info);
		::fc::detail::apply(detail::graph_adder{node_list}, conn);

		if (node_list.size() >= 2)
			for (auto it = node_list.begin() + 1; it != node_list.end(); ++it)
				current_graph.add_connection(*(it - 1), *it);

		for (auto& node : node_list)
			current_graph.add_port(node);
	}
};

/// Creates a graph_connectable with a human readable name.
template <class base_t>
auto named(base_t&& con, const std::string& name)
{
	return graph_connectable<base_t>{
			graph_node_properties{name, !has_graph_info<base_t>(0)}, std::forward<base_t>(con)};
}

} // namespace graph

template <class T>
struct is_active_sink<graph::graph_connectable<T>> : is_active_sink<T>
{
};
template <class T>
struct is_active_source<graph::graph_connectable<T>> : is_active_source<T>
{
};

} // namespace fc

#endif /* SRC_GRAPH_GRAPH_CONNECTABLE_HPP_ */
