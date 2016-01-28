#ifndef SRC_GRAPH_GRAPH_CONNECTABLE_HPP_
#define SRC_GRAPH_GRAPH_CONNECTABLE_HPP_

#include <graph/graph.hpp>
#include <core/connection.hpp>

namespace fc
{
namespace graph
{

template<class base>
struct graph_connectable : public base
{
	template<class... cstr_args>
	graph_connectable(const graph_node_properties& graph_info, const cstr_args&... args)
		: base(args...)
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

template<class connectable>
auto named(const connectable& con, const std::string& name)
{
	return graph_connectable<connectable>{graph_node_properties{name}, con};
}

template<class source_base, class sink_t>
auto connect(graph_connectable<source_base> source, sink_t sink)
{
	return make_graph_connectable(
			::fc::connect<source_base, sink_t>(source, sink),
			 source.graph_info);
}

template<class source_t, class sink_base>
auto connect(source_t source, graph_connectable<sink_base> sink)
{
	return make_graph_connectable(
			::fc::connect<source_t, sink_base>(source, sink),
			 sink.graph_info);
}

template<class source_base, class sink_base>
auto connect(graph_connectable<source_base> source,
		graph_connectable<sink_base> sink)
{
	//add edge to graph with node info of source and sink
	ad_to_graph(source.graph_info, sink.graph_info);
	return ::fc::connect<source_base, sink_base>(source, sink);
}


}  // namespace graph
template<class source_t, class sink_t>
struct result_of<graph::graph_connectable<connection<source_t, sink_t>>>
{
	typedef typename result_of<source_t>::type type;
};
}  // namespace fc

#endif /* SRC_GRAPH_GRAPH_CONNECTABLE_HPP_ */
