/*
 * port_traits.hpp
 *
 *  Created on: Sep 30, 2015
 *      Author: ckielwein
 */

#ifndef SRC_PORTS_PORT_TRAITS_HPP_
#define SRC_PORTS_PORT_TRAITS_HPP_

#include <flexcore/core/traits.hpp>

// A collection of port specific meta functions and traits.

namespace fc
{
namespace detail
{

template<class event_t>
struct handle_type
{
	using type = std::function<void(event_t)>; // need rvalue ref here?
};

template<>
struct handle_type<void>
{
	using type = std::function<void()>;
};

template <template <class...> class mixin_t, class port_t>
class is_derived_from
{
	using yes = char[1];
	using no = char[2];
	template <class... arg_t>
	static yes& test(const mixin_t<arg_t...>&);
	static no& test(...);
public:
	static constexpr bool value = sizeof(test(std::declval<port_t>())) == sizeof(yes);
};
} //namespace detail

} //namespace fc

#endif /* SRC_PORTS_PORT_TRAITS_HPP_ */
