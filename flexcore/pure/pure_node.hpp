#ifndef SRC_NODES_PURE_NODE_HPP_
#define SRC_NODES_PURE_NODE_HPP_

#include <flexcore/pure/pure_ports.hpp>

namespace fc
{

namespace pure
{

class pure_node {};

template<class port>
struct pure_port_mixin : port
{
	/**
	 * \brief forwards constructor arguments to port
	 *
	 * takes pointer to pure_node and ignores that.
	 * This means that ports with this mixin have the same signature
	 * as ports with the graph_connectable<node_aware> mixin.
	 */
	template<class... base_args>
	pure_port_mixin(pure_node*, base_args&&... args)
		: port(std::forward<base_args>(args)...)
  {
  }
};

}  // namespace pure

template <>
struct node_traits<pure::pure_node>
{
	template<class data_t>
	using event_sink = pure::pure_port_mixin<pure::event_sink<data_t>>;
	template<class data_t>
	using state_sink = pure::pure_port_mixin<pure::state_sink<data_t>>;
	template<class data_t>
	using event_source = pure::pure_port_mixin<pure::event_source<data_t>>;
	template<class data_t>
	using state_source = pure::pure_port_mixin<pure::state_source<data_t>>;
};

template<class T> struct is_active_sink<pure::pure_port_mixin<T>> : is_active_sink<T> {};
template<class T> struct is_active_source<pure::pure_port_mixin<T>> : is_active_source<T> {};

}  // namespace fc

#endif /* SRC_NODES_PURE_NODE_HPP_ */
