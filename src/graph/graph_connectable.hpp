#ifndef SRC_GRAPH_GRAPH_CONNECTABLE_HPP_
#define SRC_GRAPH_GRAPH_CONNECTABLE_HPP_

#include <graph/graph.hpp>
#include <core/connection.hpp>
#include <ports/connection_util.hpp>

namespace fc
{
namespace graph
{

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

template<class source_base_t, class sink_t>
auto connect(graph_connectable<source_base_t> source, sink_t sink)
{
	return make_graph_connectable(
			::fc::connect<source_base_t, sink_t>(source, sink),
			 source.graph_info);
}

template<class source_t, class sink_base_t>
auto connect(source_t source, graph_connectable<sink_base_t> sink)
{
	return make_graph_connectable(
			::fc::connect<source_t, sink_base_t>(source, sink),
			 sink.graph_info);
}

template<class source_base_t, class sink_base_t>
auto connect(graph_connectable<source_base_t> source,
		graph_connectable<sink_base_t> sink)
{
	//add edge to graph with node info of source and sink
	add_to_graph(get_sink(source).graph_info, get_source(sink).graph_info);
	return make_graph_connectable(
			::fc::connect<source_base_t, sink_base_t>(source, sink),
			 get_sink(source).graph_info);
}


}  // namespace graph
template<class source_t, class sink_t>
struct result_of<graph::graph_connectable<connection<source_t, sink_t>>>
{
	using type = result_of_t<source_t>;
};
}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_CONNECTABLE_HPP_ */
