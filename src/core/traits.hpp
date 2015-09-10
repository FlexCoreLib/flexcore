/*
 * traits.hpp
 *
 *  Created on: 23.02.2015
 *      Author: Caspar, tkarolski
 */

#ifndef SRC_CORE_TRAITS_H_
#define SRC_CORE_TRAITS_H_

#include <type_traits>
#include <functional>

#include "function_traits.hpp"

// is_callable trait from talesofcpp.fusionfenix.com
namespace detail
{

template<class T>
using always_void = void;

template<class Expr, class Enable = void>
struct is_callable_impl: std::false_type
{
};

template<class F, class ...Args>
struct is_callable_impl<F(Args...), always_void<std::result_of<F(Args...)>>> :std::true_type
{
};

}

/// Trait for determining if Expr is callable
template<class Expr>
struct is_callable: detail::is_callable_impl<Expr>
{
};

namespace detail {
template<class T, class enable = void>
struct has_result_impl: std::false_type
{
};

// this one will only be selected if C::result_type is valid
template<class C> struct has_result_impl<C,
	typename detail::always_void<typename C::result_type>> : std::false_type {
};
}

/// Has result trait to check if a type has a nested type 'result_type'
template<class T>
struct has_result : detail::has_result_impl<T>
{
};

/// Trait for determining the result of a callable.
/** Works on
 * - things that have a ::result_type member
 * - std::function
 * - static & member functions
 * - boost::function
 *
 * Extend this for your own types, by specializing it. Example:
 * @code
 * template<>
 * struct result_of<MyType>
 * {
 *    typedef MyType::result_type
 * }
 * @endcode */
template<class Expr, class enable = void>
struct result_of
{
	typedef  typename utils::function_traits<Expr>::result_type type;
};

template<class Expr>
struct result_of<typename std::enable_if<has_result<Expr>::value, void>>
{
	typedef typename Expr::result_type type;
};

template<class T>
struct param_type
{
	typedef typename utils::function_traits<T>::template arg<0>::type type;
};

/// Trait for determining the type of a callables parameter.
/** Works on the same types as result_of.
 *
 * Extend this for your own types, by specializing it. Example:
 * @code
 * template<>
 * struct argtype_of<MyType, 0>
 * {
 *    typedef int type;
 * }
 * @endcode
 */
template<class Expr, int Arg, class enable = void>
struct argtype_of
{
	typedef typename utils::function_traits<Expr>::template arg<Arg>::type type;
};

#endif /* SRC_CORE_TRAITS_H_ */
