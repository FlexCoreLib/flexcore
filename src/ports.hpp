#ifndef SRC_PORTS_PORTS_HPP_
#define SRC_PORTS_PORTS_HPP_

#include <extended/ports/node_aware.hpp>
#include <extended/ports/token_tags.hpp>
#include <pure/pure_ports.hpp>

#include <extended/graph/graph_connectable.hpp>

namespace fc
{

// === default mixins ===

template<class port_t>
struct node_aware_mixin : graph::graph_connectable<node_aware<port_t>>
{
	using base = graph::graph_connectable<node_aware<port_t>>;

	// TODO: this T should really be tree_base_node. It can't be because this
	// requires a full declaration of tree_base_node and tree_base_node
	// requires a full declaration of node_aware_mixin.
	template <class T, class ... args>
	node_aware_mixin(T* node_ptr, args&&... base_constructor_args)
		: base(node_ptr->graph_info(),
				*(node_ptr->region().get()),
				std::forward<args>(base_constructor_args)...)
	{
		assert(node_ptr);
	}
};

template<class T> struct is_active_sink<node_aware_mixin<T>> : is_active_sink<T> {};
template<class T> struct is_active_source<node_aware_mixin<T>> : is_active_source<T> {};

template<class port_t>
using default_mixin = node_aware_mixin<port_t>;

// -- event sinks --

template<class data_t>
using event_sink = default_mixin<pure::event_sink<data_t>>;

template<class lambda_t>
using event_sink_tmpl = default_mixin<pure::event_sink_tmpl<lambda_t>>;

template<class lambda_t>
auto make_event_sink_tmpl(lambda_t h) { return event_sink_tmpl<lambda_t>{h}; }

// -- event sources --

template<class data_t>
using event_source = default_mixin<pure::event_source<data_t>>;

// -- state sinks --

template<class data_t>
using state_sink = default_mixin<pure::state_sink<data_t>>;

// -- state sources --

template<class data_t>
using state_source = default_mixin<pure::state_source<data_t>>;

// -- dispatch --

/// template input port, tag object creates either event_sink or state_sink
template<class data_t, class tag>
struct in_port
{
	typedef default_mixin<typename pure::in_port<data_t, tag>::type> type;
};

/// template output port, tag object creates either event_source or state_source
template<class data_t, class tag>
struct out_port
{
	typedef default_mixin<typename pure::out_port<data_t, tag>::type> type;
};

} // namespace fc

#endif /* SRC_PORTS_PORTS_HPP_ */
