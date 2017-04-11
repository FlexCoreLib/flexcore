#ifndef SRC_CORE_DETAIL_CONNECTION_HPP_
#define SRC_CORE_DETAIL_CONNECTION_HPP_

#include <type_traits>

#include <flexcore/core/detail/function_traits.hpp>
#include <flexcore/core/traits.hpp>

/// All Classes, Functions and other symbols from flexcore are in this namespace.
namespace fc
{

namespace detail
{

enum void_flag
{
	payload_void,
	payload_not_void
};

template<class source,class sink, class... P>
constexpr auto void_check() ->  decltype(std::declval<sink>()(), std::declval<source>()(std::declval<P>()...), void_flag())
{
	return payload_void;
}

template<class source,class sink, class... P>
constexpr auto void_check() ->  decltype(std::declval<sink>()(std::declval<source>()(std::declval<P>()...)), void_flag())
{
	return payload_not_void;
}

template<void_flag vflag, class source_t, class sink_t, class... param>
struct invoke_helper
{
};

template<class source_t, class sink_t>
struct invoke_helper<payload_void, source_t, sink_t>
{
	template<class... param>
	constexpr decltype(auto) operator()(source_t& source, sink_t& sink, param&&... p) const
	{
		source(std::forward<param>(p)...);
		return sink();
	}
};

template<class source_t, class sink_t>
struct invoke_helper<payload_not_void, source_t, sink_t>
{
	template<class... param>
	constexpr decltype(auto) operator()(source_t& source, sink_t& sink, param&&... p) const
	{
		return sink(source(std::forward<param>(p)...));
	}
};

template <bool source_void_callable, class source_t, class sink_t>
struct void_check_signatures_impl;
template <class source_t, class sink_t>
struct void_check_signatures_impl<false, source_t, sink_t>
{
	// can't check now so return true
	constexpr bool operator()() { return true; }
};
template <class source_t, class sink_t>
struct void_check_signatures_impl<true, source_t, sink_t>
{
	constexpr bool operator()()
	{
		bool sink_void_callable = void_callable<sink_t>(0);
		bool source_compatible = detail::has_result_of_type<sink_t, decltype(std::declval<source_t>()())>();
		return sink_void_callable || source_compatible;
	}
};

} //namespace detail

/// check whether the signatures of the connectables are compatible in the
/// case of void_callable source (the only one that is easy to check).
template <class source_t, class sink_t>
constexpr bool void_check_signatures()
{
	return detail::void_check_signatures_impl<void_callable<source_t>(0), source_t, sink_t>{}();
}
/**
 * \brief defines basic connection object, which is connectable.
 * \tparam source_t the source of the connection, data flows from here to the sink.
 * \tparam sink_t is the sink of the connection, data goes here.

 * \pre the return value of source_t needs to be convertible to the parameter of sink_t.
 */
template<class source_t, class sink_t>
struct connection
{
	source_t source;
	sink_t sink;
	static_assert(void_check_signatures<source_t, sink_t>(),
	              "The return type of source is incompatible with parameter type of sink.");

	/**
	 * \brief call operator, calls source and then sink with the result of source
	 *
	 * redundant return type specification is necessary at the moment,
	 * as it causes missing operator() in source and sink to appear in function declaration
	 * and thus be a substitution failure.
	 */
	template<class S = source_t, class T = sink_t, class... param>
	constexpr auto operator()(param&&... p)
	-> decltype(detail::invoke_helper<
					detail::void_check<S, T, param...>(),
					S,T
				>()
			(source, sink, std::forward<param>(p)...))
	{
		constexpr auto test = detail::void_check<S, T, param...>();
		return detail::invoke_helper<
					test,
					S,T
				>()
				(source, sink, std::forward<param>(p)...);
	}

	///const overload of call operator
	template<class S = source_t, class T = sink_t, class... param>
	constexpr auto operator()(param&&... p) const
	-> decltype(detail::invoke_helper<
				detail::void_check<const S, const T, param...>(),
				const S, const T
			>()
			(source, sink, std::forward<param>(p)...))
	{
		constexpr auto test = detail::void_check<const S, const T, param...>();
		return detail::invoke_helper<
					test,
					const S, const T
				>()
				(source, sink, std::forward<param>(p)...);
	}
};

/// internals of flexcore, everything here can change any time.
namespace detail
{

template<class source_t, class sink_t, class Enable = void>
struct connect_impl
{
	constexpr auto operator()(source_t&& source, sink_t&& sink)
	{
		return connection<source_t, sink_t>
				{std::forward<source_t>(source), std::forward<sink_t>(sink)};
	}
};

} // namespace detail

template <typename T>
using rm_ref_t = std::remove_reference_t<T>;

/**
 * \brief Connect takes two connectables and returns a connection.
 *
 * \param source is the source of the data flowing through the connection.
 * \param sink is the target of the data flowing through the connection.
 * \returns connection object which has its type determined by the source_t and sink_t.
 *
 * If source_t and sink_t fulfill connectable, the result is connectable.
 * If one of source_t and sink_t fulfills passive_connectable and the other
 * fulfills active_connectable, the result is a complete connection and non_connectable.
 * If either source_t or sink_t fulfill active_connectable, the result is an active connection proxy.
 * If either source_t or sink_t fulfill passive_connectable, the result is passive_connectable.
 */
template<class source_t, class sink_t, class enable = std::enable_if_t<
		(is_connectable_v<source_t> || is_active_source_v<rm_ref_t<source_t>>)
		&& (is_connectable_v<sink_t> || is_active_sink_v<rm_ref_t<sink_t>>)>>
constexpr auto connect (source_t&& source, sink_t&& sink)
{
	return detail::connect_impl<source_t, sink_t>()(
	        std::forward<source_t>(source), std::forward<sink_t>(sink));
}

/**
 * \brief Operator >> takes two connectables and returns a connection.
 *
 * This operator is syntactic sugar for connect.
 */
template<class source_t, class sink_t, class enable = std::enable_if_t<
		(is_connectable_v<source_t> || is_active_source_v<rm_ref_t<source_t>>)
		&& (is_connectable_v<sink_t> || is_active_sink_v<rm_ref_t<sink_t>>)>>
constexpr auto operator >>(source_t&& source, sink_t&& sink)
{
	static_assert(!(is_active_v<rm_ref_t<source_t>> && is_active_v<rm_ref_t<sink_t>>),
	              "event_source can not be connected to state_sink.");
	return connect(std::forward<source_t>(source), std::forward<sink_t>(sink));
}

template <class source_t, class sink_t>
struct is_passive_sink<connection<source_t, sink_t>> : is_passive_sink<rm_ref_t<sink_t>>
{};

template <class source_t, class sink_t>
struct result_of<connection<source_t, sink_t>>
	: std::result_of<sink_t(result_of_t<rm_ref_t<source_t>>)>
{};

} //namespace fc

#endif /* SRC_CORE_CONNECTION_HPP_ */
