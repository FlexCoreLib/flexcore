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
	typedef std::function<void(event_t)> type; // need rvalue ref here?
};

template<>
struct handle_type<void>
{
	typedef std::function<void()> type;
};

} //namespace detail

} //namespace fc

#endif /* SRC_PORTS_PORT_TRAITS_HPP_ */
