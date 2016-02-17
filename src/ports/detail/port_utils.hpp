/*
 * port_utils.hpp
 *
 *  Created on: Feb 15, 2016
 *      Author: jpiotrowski
 */
#ifndef SRC_PORTS_PORT_UTILS_HPP_
#define SRC_PORTS_PORT_UTILS_HPP_

namespace fc
{
namespace detail
{

template <class conn_t>
auto handler_wrapper(conn_t& c)
{
	return [&c](auto&&... args)
	{
		return c(std::forward<decltype(args)>(args)...);
	};
}
template <class conn_t, class enable = std::enable_if_t<std::is_rvalue_reference<conn_t&&>{}>>
decltype(auto) handler_wrapper(conn_t&& c)
{
	return std::move(c);
}

} //namespace detail
} //namespace fc

#endif /* SRC_PORTS_PORT_UTILS_HPP_ */
