/*
 * traits.hpp
 *
 *  Created on: 23.02.2015
 *      Author: Caspar, tkarolski
 */

#ifndef SRC_CORE_TRAITS_H_
#define SRC_CORE_TRAITS_H_

#include <functional>
#include <memory>
#include <type_traits>

#include <flexcore/core/detail/function_traits.hpp>

namespace fc
{

/**
	\defgroup traits traits
	\brief metaprogramming traits used in flexcore

	This group contains all traits and metafunctions which provided by
	and used in flexcore.
*/
namespace detail
{

template<class T>
using always_void = void;
template <typename T>
void always_void_fun();

template<class Expr, class Enable = void>
struct expr_is_callable_impl: std::false_type
{
};

template<class F, class ...Args>
struct expr_is_callable_impl<F(Args...), always_void<std::result_of<F(Args...)>>> :std::true_type
{
};

/** \addtogroup traits
 *  @{
 */

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

	// in all other caes, which means, T has operator() with any argument,
	// this overload will be chosen and the return is true_type.
	template<class>
	static std::true_type test(...);

public:
	// this actually evaluates the test by building the derived type
	// and evaluating the result type of test, either false_type or true_type
	using type = decltype(test<derived>(nullptr));
};


template<class,int> struct argtype_of;
template<class T>
struct type_is_callable_impl : has_call_op<T>::type
{
};

template <class result_of>
constexpr auto has_result_of_type_impl(int) -> decltype(always_void_fun<typename result_of::type>(), true)
{
	return true;
}
template <class result_of>
constexpr bool has_result_of_type_impl(...)
{
	return false;
}
template <class F, typename... Args>
struct result_of_fwd
{
	using type = std::result_of<F(Args...)>;
};
template <class F>
struct result_of_fwd<F, void>
{
	using type = std::result_of<F()>;
};

/**
 * \brief Check whether F can be called with Arg.
 *
 * Is done by checking whether std::result_of has a type alias.
 */
template <class F, typename... Arg>
constexpr bool has_result_of_type()
{
	return has_result_of_type_impl<typename result_of_fwd<F, Arg...>::type>(0);
}

} // namespace detail

/// Trait for determining if Expr is callable
template<class Expr>
struct is_callable:
		std::conditional_t<
			std::is_class<Expr>::value,
			detail::type_is_callable_impl<Expr>,
			detail::expr_is_callable_impl<Expr>
		>
{
};

template<class T>
using is_callable_t = typename is_callable<T>::type;
template<class T>
constexpr bool is_callable_v = is_callable<T>::value;

/**
 *  \brief Checks if type T is connectable.
 *
 *  This requires to to be callable
 *  and to be either an lvalue reference or copy constructible.
 */
template <class T>
struct is_connectable :
	std::integral_constant<bool,
	    is_callable_v<std::remove_reference_t<T>> &&
	    (std::is_lvalue_reference<T>::value || std::is_copy_constructible<T>::value)
	>
{
};

template<class T>
using is_connectable_t = typename is_connectable<T>::type;
template<class T>
constexpr bool is_connectable_v = is_connectable<T>::value;

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

namespace detail
{
template<class, bool> struct result_of_impl;

template<class Expr>
struct result_of_impl<Expr, false>
{
using type = typename utils::function_traits<Expr>::result_t;
};

template<class Expr>
struct result_of_impl<Expr, true>
{
using type = typename std::remove_reference_t<Expr>::result_t;
};
} //namespace detail
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
 *    using type = MyType::result_t
 * }
 * @endcode */
template<class Expr>
struct result_of
{
	using type = typename detail::result_of_impl<
			Expr, has_result<std::remove_reference_t<Expr>>::value>::type;
};
template <class Expr>
using result_of_t = typename result_of<Expr>::type;

/// Trait for determining the type of a callables parameter.
/** Works on the same types as result_of.
 *
 * Extend this for your own types, by specializing it. Example:
 * @code
 * template<>
 * struct argtype_of<MyType, 0>
 * {
 *     using type = int;
 * }
 * @endcode
 */
template<class Expr, int Arg, class enable = void>
struct argtype_of
{
	using type = typename utils::function_traits<Expr>::template arg<Arg>::type;
};

template<class T>
struct param_type
{
	using type = typename argtype_of<T,0>::type;
};


///helper type to have always false values in sfinae and static_asserts.
template <class T>
struct always_false : std::false_type
{
};

///Checks if type T can be called with void
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

///Checks if type T has a member function register_callback
template <class T>
constexpr auto has_register_function(int)
    -> decltype(std::declval<T>().register_callback(
                    std::declval<std::shared_ptr<std::function<void(size_t)>>&>()),
                bool())
{
	return true;
}

template<class T>
constexpr bool has_register_function(...)
{
	return false;
}

///Checks if type T has a member source
template <class T>
constexpr bool has_source(...)
{
	return false;
}

template <class T>
constexpr auto has_source(int) -> decltype(
		std::declval<T>().source, bool())
{
	return true;
}

///Checks if type T has a member sink
template <class T>
constexpr bool has_sink(...)
{
	return false;
}

template <class T>
constexpr auto has_sink(int) -> decltype(
		std::declval<T>().sink, bool())
{
	return true;
}

template <class T>
constexpr auto has_token_type(int) -> decltype(std::declval<typename T::token_t>(), bool())
{
	return true;
}
template <class T>
constexpr bool has_token_type(...)
{
	return false;
}

///Checks if type T has an overloaded call operator
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

/**
 * \brief Trait to check if type T instantiates a template
 * \tparam T type to check
 * \tparam template_type template T is asked to instantiate
 */
template<template<class ...> class template_type, class T >
struct is_instantiation_of : std::false_type
{};

template<template<class ...> class template_type, class... Args >
struct is_instantiation_of< template_type, template_type<Args...> > : std::true_type {};


//************* Checks if types are active or passive connectables ************/
namespace detail
{
template<class T, class enable = void>
struct is_passive_source_impl: std::false_type
{
};

template<class T>
struct is_passive_source_impl<T, std::enable_if_t<void_callable<T>(0)>> : std::true_type
{
};

template<class T, class enable = void>
struct is_passive_sink_impl: std::false_type
{
};

template<class T>
struct is_passive_sink_impl<T, std::enable_if_t<is_callable_v<T>
			&& !overloaded<T>(0)>>
		: std::integral_constant<bool, std::is_void<result_of_t<T>>::value>
{
};

template<class T>
struct is_passive_sink_impl<T, std::enable_if_t<is_callable_v<T>
			&& overloaded<T>(0)
			&& has_result<T>::value>>
		: std::integral_constant<bool, std::is_void<result_of_t<T>>::value>
{
};
} //namespace detail

template <class T, class Arg, typename = void>
struct is_passive_sink_for : std::false_type {};
template <class T>
struct is_passive_sink_for<T, void, std::result_of_t<T()>> : std::true_type {};
template <class T, class Arg>
struct is_passive_sink_for<T, Arg, std::result_of_t<T(Arg)>> : std::true_type {};

/**
 * \brief Checks if type T is a passive sink.
 *
 * It checks if the type has a call operator which returns void.
 */
template<class T>
struct is_passive_sink: detail::is_passive_sink_impl<T>
{
};

template<class T>
using is_passive_sink_t = typename is_passive_sink<T>::type;
template<class T>
constexpr bool is_passive_sink_v = is_passive_sink<T>::value;
template<class T>
constexpr bool class_is_passive_sink_v = is_passive_sink_v<std::decay_t<T>>;
/**
 * \brief Checks if type T is a passive sink.
 *
 * It checks if the type has a call operator which takes void.
 */
template<class T>
struct is_passive_source: detail::is_passive_source_impl<T>
{
};

template<class T>
using is_passive_source_t = typename is_passive_source<T>::type;
template<class T>
constexpr bool is_passive_source_v = is_passive_source<T>::value;
template<class T>
constexpr bool class_is_passive_source_v = is_passive_source_v<std::decay_t<T>>;

///checks if type T is either a passive sink or a passive source.
template<class T>
struct is_passive: std::integral_constant<bool,
		is_passive_source_v<T> || is_passive_sink_v<T>>
{
};

/**
 * \brief Trait to define an active sink.
 *
 * define is_active_sink<my_type> : std::true_type
 * for your own types, if they are active sinks.
 */
template<class T>
struct is_active_sink: std::false_type
{
};

template<class T>
using is_active_sink_t = typename is_active_sink<T>::type;
template<class T>
constexpr bool is_active_sink_v = is_active_sink<T>::value;

///shortcut for is_active which decays template parameter
template<class T>
constexpr bool class_is_active_sink_v = is_active_sink_v<std::decay_t<T>>;
/**
 * \brief  Trait to define an active source.
 *
 * define is_active_source<my_type> : std::true_type
 * for your own types, if they are active sources.
 */
template<class T>
struct is_active_source: std::false_type
{
};

template<class T>
using is_active_source_t = typename is_active_source<T>::type;
template<class T>
constexpr bool is_active_source_v = is_active_source<T>::value;

///shortcut for is_active which decays template parameter
template<class T>
constexpr bool class_is_active_source_v = is_active_source_v<std::decay_t<T>>;
/// Checks if type T is either active_source or active_sink
template<class T>
struct is_active: std::integral_constant<bool,
is_active_source_v<T> || is_active_sink_v<T>>
{
};

template<class T>
using is_active_t = typename is_active<T>::type;
template<class T>
constexpr bool is_active_v = is_active<T>::value;

///shortcut for is_active which decays template parameter
template<class T>
constexpr bool class_is_active_v = is_active_v<std::decay_t<T>>;

template<class T>
struct is_event_port: std::integral_constant<bool,
is_active_source_v<T> || is_passive_sink_v<T>>
{
};

template<class T>
using is_event_port_t = typename is_event_port<T>::type;
template<class T>
constexpr bool is_event_port_v = is_event_port<T>::value;

template<class T>
struct is_state_port: std::integral_constant<bool,
	is_active_sink_v<T> || is_passive_source_v<T>>
{
};

template<class T>
using is_state_port_t = typename is_state_port<T>::type;
template<class T>
constexpr bool is_state_port_v = is_state_port<T>::value;
/** @}*/ //doxygen group traits

} // namespace fc

#endif /* SRC_CORE_TRAITS_H_ */
