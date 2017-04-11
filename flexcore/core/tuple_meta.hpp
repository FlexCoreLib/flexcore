/*
 * tuple_meta.hpp
 *
 *  Created on: Nov 4, 2015
 *      Author: ckielwein
 */

#ifndef SRC_CORE_TUPLE_META_HPP_
#define SRC_CORE_TUPLE_META_HPP_

#include <tuple>
#include <utility>

namespace fc
{

/**
 * \brief Selection of metafunctions using std::tuple.
 * Basically a very minimal subset of boost::fusion
 */
namespace tuple
{

namespace detail
{
template <class tuple>
constexpr auto make_index_sequence()
{
	return std::make_index_sequence<std::tuple_size<std::decay_t<tuple>>::value>{};
}

template<class lhs_tuple, class rhs_tuple, std::size_t ... index,
class operation>
constexpr decltype(auto) binary_invoke_helper(lhs_tuple&& lsh,
		rhs_tuple&& rhs,
		std::index_sequence<index...>,
		operation&& op)
{
	return std::make_tuple(op(
					std::get<index>(std::forward<lhs_tuple>(lsh)),
					std::get<index>(std::forward<rhs_tuple>(rhs)))...);
}

template<class lhs_tuple, std::size_t ... index,
class operation>

constexpr decltype(auto) unary_invoke_helper(lhs_tuple&& lsh,
		std::index_sequence<index...>,
		operation&& op)
{
	return std::make_tuple(op(std::get<index>(std::forward<lhs_tuple>(lsh)))...);
}

template<class operation, class tuple, std::size_t... index>
constexpr decltype(auto) invoke_function_helper(
		operation&& op, tuple&& tup, std::index_sequence<index...>)
{
	return op(std::get<index>(std::forward<tuple>(tup))...);
}
} //namespace detail

///applies function to every element in tuple
template<class tuple, class operation>
constexpr void for_each(tuple&& tup, operation&& op)
{
	unary_invoke_helper(std::forward<tuple>(tup), detail::make_index_sequence<tuple>(), op);
}

///transform, returns tuple of transformed elements
template<class tuple, class operation>
constexpr decltype(auto) transform(tuple&& tup, const operation& op)
{
	return detail::unary_invoke_helper(std::forward<tuple>(tup),
	                                   detail::make_index_sequence<tuple>(), op);
}
///binary_transform, returns tuple of results of bin_op on elements of first and second tuple
template<class first_tuple, class second_tuple, class operation>
constexpr decltype(auto) transform(first_tuple&& first, second_tuple&& second, const operation& op)
{
	static_assert(std::tuple_size<std::decay_t<first_tuple>>::value ==
	                  std::tuple_size<std::decay_t<second_tuple>>::value,
	              "Binary tuple transform needs tuples to have same nr of elements.");

	return detail::binary_invoke_helper(std::forward<first_tuple>(first),
	                                    std::forward<second_tuple>(second),
	                                    detail::make_index_sequence<first_tuple>(), op);
}

///Helper function to call variadic functions with tuple
template<class operation, class tuple>
constexpr decltype(auto) invoke_function(operation&& op, tuple&& tup)
{
	return detail::invoke_function_helper(
			std::forward<operation>(op),
			std::forward<tuple>(tup),
			detail::make_index_sequence<tuple>());
}

}  // namespace tuple
}  // namespace fc

#endif /* SRC_CORE_TUPLE_META_HPP_ */
