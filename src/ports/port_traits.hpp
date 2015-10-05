/*
 * port_traits.hpp
 *
 *  Created on: Sep 30, 2015
 *      Author: ckielwein
 */

#ifndef SRC_PORTS_PORT_TRAITS_HPP_
#define SRC_PORTS_PORT_TRAITS_HPP_

#include <core/traits.hpp>

// A collection of port specific meta functions and traits.

namespace fc
{
namespace detail
{

template<class event_t>
struct handle_type
{
	typedef std::function<void(event_t)> type;
};

template<>
struct handle_type<void>
{
	typedef std::function<void()> type;
};

} //namespace detail

template<class T>
struct is_port : std::false_type
{
};


template<class T>
struct is_active_source:
		std::integral_constant<bool,
		is_active_connectable<T>::value &&
		is_port<T>::value>

{
};

// connection type, when to ports are connected, has no runtime information or access to the ports.
template<class source, class sink>
struct port_connection
{
	typedef source source_t;
	typedef sink sink_t;
};

} //namespace fc

#endif /* SRC_PORTS_PORT_TRAITS_HPP_ */
