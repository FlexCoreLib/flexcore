/*
 * port_utils.hpp
 *
 *  Created on: Feb 15, 2016
 *      Author: jpiotrowski
 */
#ifndef SRC_PORTS_PORT_UTILS_HPP_
#define SRC_PORTS_PORT_UTILS_HPP_

#include <functional>

namespace fc
{
namespace detail
{

// std::function requires that the stored object be copyable. Once we arrive at the active port's
// connect member function, the argument is either an rvalue copyable object or an lvalue
// potentially move only type. So for lvalues it is necessary to construct a copyable wrapper
// (forwarding lambda or std::ref will do).
template <class conn_t>
auto handler_wrapper(conn_t& c)
{
	return std::ref(c);
}
template <class conn_t, class enable = std::enable_if_t<std::is_rvalue_reference<conn_t&&>{}>>
decltype(auto) handler_wrapper(conn_t&& c)
{
	return std::move(c);
}

} //namespace detail
} //namespace fc

#endif /* SRC_PORTS_PORT_UTILS_HPP_ */
