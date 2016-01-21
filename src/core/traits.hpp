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
#include <memory>

#include "function_traits.hpp"

namespace fc
{

namespace detail
{

template<class T>
using always_void = void;

template<class Expr, class Enable = void>
struct expr_is_callable_impl: std::false_type
{
};

template<class F, class ...Args>
struct expr_is_callable_impl<F(Args...), always_void<std::result_of<F(Args...)>>> :std::true_type
{
};


/**
 * \brief has_call_op trait, based on member_detector idiom
 *
 * Explanation found at: https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
 *
 * This trait is used by result_of metafunction to check if a type has operator().
 */
template<class T>
struct has_call_op
{
private:
	struct fallback
	{
		void operator()();
	};
	// derived always has one operator() (the one from fallback)
	// if T has operator(), derived will have that one as well and lookup will take it first.
	struct derived : T, fallback
	{
	};

	template<class U, U> struct check;

	/*
	 * if T has no(!) operator(),
	 *  void (fallback::*)() and  &to_test::operator()>*, will be the same.
	 *  Then this overload of test will be chosen, and the return value will be false_type
	 */
	template<class to_test>
	static std::false_type test(check<void (fallback::*)(), &to_test::operator()>*);

	// in all other cases, which means, T has operator() with any argument,
	// this overload will be chosen and the return is true_type.
	template<class>
	static std::true_type test(...);

public:
	// this actually evaluates the test by building the derived type
	// and evaluating the result type of test, either false_type or true_type
	typedef decltype(test<derived>(nullptr)) type;
};

/// has_member check for method connect, see has_call_op for explanation
template<class T>
struct has_member_connect
{
private:
	struct fallback
	{
		void connect();
	};

	struct derived : T, fallback
	{
	};

	template<class U, U> struct check;

	template<class to_test>
	static std::false_type test(check<void (fallback::*)(), &to_test::connect>*);

	template<class>
	static std::true_type test(...);

public:
	typedef decltype(test<derived>(nullptr)) type;
};

template<class,int> struct argtype_of;
template<class T>
struct type_is_callable_impl : has_call_op<T>::type
{
};

} // namespace detail

/// Trait for determining if Expr is callable
template<class Expr>
struct is_callable:
		std::conditional<
			std::is_class<Expr>::value,
			detail::type_is_callable_impl<Expr>,
			detail::expr_is_callable_impl<Expr>
		>::type
{
};

template<class T>
struct is_connectable
{
	static const bool value =
			is_callable<T>::value &&
			std::is_copy_constructible<T>::value;
};

namespace detail
{
template<class T>
static std::false_type has_result_impl(T*);

template<class T>
static std::true_type has_result_impl(typename T::result_t*);

}

/// Has result trait to check if a type has a nested type 'result_t'
template<class T>
struct has_result : decltype(detail::has_result_impl<T>(nullptr))
{
};

namespace detail{ template<class, bool> struct result_of_impl; }
/// Trait for determining the result of a callable.
/** Works on
 * - things that have a ::result_t member
 * - std::function
 * - static & member functions
 * - boost::function
 *
 * Extend this for your own types, by specializing it. Example:
 * @code
 * template<>
 * struct result_of<MyType>
 * {
 *    typedef MyType::result_t
 * }
 * @endcode */
template<class Expr>
struct result_of
{
	typedef typename detail::result_of_impl<Expr,
			has_result<typename std::remove_reference<Expr>::type>::value>::type type;
};

namespace detail
{
template<class Expr>
struct result_of_impl<Expr, false>
{
	typedef typename utils::function_traits<Expr>::result_t type;
};

template<class Expr>
struct result_of_impl<Expr, true>
{
	typedef typename std::remove_reference<Expr>::type::result_t type;
};
} //namespace detail
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

template<class T>
struct param_type
{
	typedef typename argtype_of<T,0>::type type;
};

template<class T>
struct is_active_sink: public std::false_type
{
};

template<class T>
struct is_active_source: public std::false_type
{
};

template<class T, class enable = void>
struct is_active_connectable_impl : std::false_type
{
};

template<class T>
struct is_active_connectable_impl<
	T, typename std::enable_if<std::is_class<T>::value>::type> :
		std::integral_constant
			<	bool,
					detail::has_member_connect<T>::type::value
				and	std::is_copy_constructible<T>::value
			>
{
};
template<class T>
struct is_active_connectable : is_active_connectable_impl<T>
{
};

template<class T, class enable = void>
struct is_passive_source_impl: public std::false_type
{
};

template<class T>
constexpr auto void_callable(int) -> decltype(std::declval<T>()(), bool())
{
	return true;
}

template<class T>
constexpr bool void_callable(...)
{
	return false;
}

template<class T>
constexpr auto has_register_function(int) -> decltype(
			std::declval<T>().register_callback(std::make_shared<std::function<void(void)>>()),
		bool())
{
	return true;
}

template<class T>
constexpr bool has_register_function(...)
{
	return false;
}

//template<class T>
//struct is_passive_source_impl<T,typename std::enable_if<is_callable<T>::value>::type>
//		: public std::integral_constant<bool, void_callable<T>(0)>
//{
//};

template<class T>
struct is_passive_source_impl<T,typename std::enable_if<void_callable<T>(0)>::type>
		: public std::integral_constant<bool, true>
{
};

template<class T, class enable = void>
struct is_passive_sink_impl: public std::false_type
{
};

template<class T>
constexpr auto overloaded(int) -> decltype(&T::operator(), bool())
{
	//in case operator() is overloaded decltype will fail, and this template will not be instantiated.
	return false;
}

template<class T>
constexpr bool overloaded(...)
{
	return true;
}

template<class T>
struct is_passive_sink_impl<T,typename std::enable_if<is_callable<T>::value
			&& !overloaded<T>(0)>::type>
		: public std::integral_constant<bool, std::is_void<typename result_of<T>::type>::value>
{
};

template<class T>
struct is_passive_sink_impl<T,typename std::enable_if<is_callable<T>::value
			&& overloaded<T>(0)
			&& has_result<T>::value>::type>
		: public std::integral_constant<bool, std::is_void<typename result_of<T>::type>::value>
{
};


template<class T>
struct is_passive_sink: public is_passive_sink_impl<T>
{
};
template<class T>
struct is_passive_source: public is_passive_source_impl<T>
{
};


//todo cleanup of diverse redundant traits
template<class T>
struct is_passive: std::integral_constant<bool,
		is_passive_source<T>::value || is_passive_sink<T>::value>
{
};

//todo cleanup of diverse redundant traits
template<class T>
struct is_active: std::integral_constant<bool,
is_active_connectable<T>::value || is_active_sink<T>::value>
{
};

template<template<class ...> class template_type, class T >
struct is_instantiation_of : std::false_type
{};

template<template<class ...> class template_type, class... Args >
struct is_instantiation_of< template_type, template_type<Args...> > : std::true_type {};

} // namespace fc

#endif /* SRC_CORE_TRAITS_H_ */
