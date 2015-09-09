#ifndef SRC_CORE_TRAITS_HPP_
#define SRC_CORE_TRAITS_HPP_

#include <type_traits>
#include <functional>

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


//template<class Expr>
//struct is_connectable : detail::is_connectable_impl<Expr>::type{
//};


namespace detail {
template<class T, class enable = void>
struct has_result_impl: std::false_type {
};
// this one will only be selected if C::result_type is valid
template<class C> struct has_result_impl<C,
	typename detail::always_void<typename C::result_type>> : std::false_type {
};
}

// Has result trait to check if a type has a nested type result_type
template<class T>
struct has_result : detail::has_result_impl<T>{
};

template<class Expr, class enable = void>
struct result_of {
	typedef  typename utils::function_traits<Expr>::result_type type;
};

template<class Expr>
struct result_of<typename std::enable_if<has_result<Expr>::value, void>>{
	typedef typename Expr::result_type type;
};

template<class T>
using ParamType = typename utils::function_traits<T>::template arg<0>::type;

#endif /* SRC_CORE_TRAITS_HPP_ */
