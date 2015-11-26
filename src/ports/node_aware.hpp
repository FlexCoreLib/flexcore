#ifndef SRC_PORTS_NODE_AWARE_HPP_
#define SRC_PORTS_NODE_AWARE_HPP_

#include <ports/port_traits.hpp>
#include <core/connection.hpp>
#include <core/traits.hpp>
#include <nodes/node_interface.hpp>

namespace fc
{

template<class base>
struct node_aware: public base
{
	static_assert(std::is_class<base>::value,
			"can only be mixed into clases, not primitives");
	//allows explicit access to base of this mixin.
	typedef base base_t;

	template<class ... args>
	node_aware( node_interface* node_,
				const args& ... base_constructor_args)
		: base_t(base_constructor_args...)
		, node(node_)
	{
		assert(node);
	}

	node_interface* node;
};

// TODO prefer to test this algorithmically
template<class T> struct is_port<node_aware<T>> : public std::true_type {};
template<class T> struct is_active_sink<node_aware<T>> : public is_active_sink<T> {};
template<class T> struct is_active_source<node_aware<T>> : public is_active_source<T> {};
template<class T> struct is_passive_sink<node_aware<T>> : public is_passive_sink<T> {};
template<class T> struct is_passive_source<node_aware<T>> : public is_passive_source<T> {};

} // namespace fc

#endif /* SRC_PORTS_NODE_AWARE_HPP_ */
