#ifndef SRC_PORTS_PORTS_HPP_
#define SRC_PORTS_PORTS_HPP_

#include <flexcore/extended/node_fwd.hpp>
#include <flexcore/extended/ports/node_aware.hpp>
#include <flexcore/extended/ports/token_tags.hpp>
#include <flexcore/extended/graph/graph_connectable.hpp>
#include <flexcore/pure/pure_ports.hpp>

namespace fc
{

/**
 * \brief mixin for ports, which makes them aware of parent node and available in graph.
 *
 * Use these ports together with tree_base_node and owning_base_node.
 * \ingroup ports
 */
template<class port_t>
struct node_aware_mixin : graph::graph_connectable<node_aware<port_t>>
{
	using base = graph::graph_connectable<node_aware<port_t>>;

	/**
	 * \brief Constructs port with node_aware mixin.
	 * \param node_ptr pointer to node which owns this port
	 * \pre node_ptr != nullptr
	 * \param base_constructor_args constructor arguments to underlying port.
	 * These are forwarded to base
	 */
	template <class ... args>
	explicit node_aware_mixin(node* node_ptr, args&&... base_constructor_args)
			: base(node_ptr->get_graph(), node_ptr->graph_info(),
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

/**
 * \brief Default event_sink port
 * \ingroup ports
 */
template<class data_t>
using event_sink = default_mixin<pure::event_sink<data_t>>;

/**
 * \brief Default event_source port
 * \ingroup ports
 */
template<class data_t>
using event_source = default_mixin<pure::event_source<data_t>>;

/**
 * \brief Default state_sink port
 * \ingroup ports
 */
template<class data_t>
using state_sink = default_mixin<pure::state_sink<data_t>>;

/**
 * \brief Default state_source port
 * \ingroup ports
 */
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
