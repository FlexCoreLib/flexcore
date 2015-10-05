#ifndef SRC_PORTS_NODE_AWARE_PORT_HPP_
#define SRC_PORTS_NODE_AWARE_PORT_HPP_

#include <ports/port_traits.hpp>
#include <core/connection.hpp>

class node_id;
//todo

namespace fc
{

template<class port_t>
struct node_aware_port: public port_t
{
	static_assert(is_port<port_t>::value,
			"node_aware_port can only be mixed to port types");
	//allows explicit access to base of this mixin.
	typedef port_t base_t;

	template<class ... args>
	node_aware_port(args ... base_constructor_args) :
			port_t(base_constructor_args...)
	{

	}

	std::function<void()> switch_tick;
	std::function<void()> send_tick;
};

template<class base_connection>
struct node_aware_connection : public base_connection
{
	node_aware_connection(const base_connection base) :
		base_connection(base)
	{
	}
};

// TODO prefer to test this algorithmically
template<class T> struct is_port<node_aware_port<T>> : public std::true_type
{
};

namespace detail
{
template<class sink_t, class source_t>
struct node_aware_connect_impl
{
	auto operator()(source_t source, sink_t sink)
	{
		const auto base = static_cast<typename source_t::base_t>(source);
		typedef decltype(::fc::connect(base, sink)) base_t;
		return node_aware_connection<base_t>(::fc::connect(base, sink));
	}
};

}  //namespace detail

template<class source_t, class sink_t>
auto connect(node_aware_port<source_t> source, sink_t sink)
{
	return detail::node_aware_connect_impl<sink_t, node_aware_port<source_t>>()
			(source, sink);
}

template<class base_connection, class sink_t>
auto connect(node_aware_connection<base_connection> source, sink_t sink)
{
	return connect(static_cast<base_connection>(source), sink);
}


}  //namespace fc

#endif /* SRC_PORTS_NODE_AWARE_PORT_HPP_ */
