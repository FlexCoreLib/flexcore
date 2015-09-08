/*
 * traits.hpp
 *
 *  Created on: Sep 8, 2015
 *      Author: ckielwein
 */

#ifndef SRC_CORE_TRAITS_HPP_
#define SRC_CORE_TRAITS_HPP_

#include <type_traits>
#include <functional>

#include "functional.h"

// is_callable trait
namespace detail {
template<class T>
using always_void = void;

template<class Expr, class Enable = void>
struct is_callable_impl: std::false_type {
};

template<class F, class ...Args>
struct is_callable_impl<F(Args...), always_void<std::result_of<F(Args...)>>> :std::true_type
{};
}

template<class Expr>
struct is_callable: detail::is_callable_impl<Expr> {
};


template<class Expr>
struct is_connectable : detail::is_connectable_impl<Expr>::type{
};

#endif /* SRC_CORE_TRAITS_HPP_ */
