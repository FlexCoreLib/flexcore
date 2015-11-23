#ifndef SRC_CORE_DETAIL_CONNECTION_HPP_
#define SRC_CORE_DETAIL_CONNECTION_HPP_

#include <type_traits>

#include "core/function_traits.hpp"
#include "core/traits.hpp"

namespace fc
{

namespace detail
{

enum void_flag
{
	payload_void,
	payload_not_void,
	invalid,
};

template<class source,class sink, class... P>
constexpr auto void_check(int) ->  decltype(std::declval<sink>()(), std::declval<source>()(std::declval<P>()...), void_flag())
{
	return payload_void;
}

template<class source,class sink, class... P>
constexpr auto void_check(int) ->  decltype(std::declval<sink>()(std::declval<source>()(std::declval<P>()...)), void_flag())
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
	decltype(auto) operator()(source_t& source, sink_t& sink, param&&... p)
	{
		source(p...);
		return sink();
	}
};

template<class source_t, class sink_t>
struct invoke_helper<payload_not_void, source_t, sink_t>
{
	template<class... param>
	decltype(auto) operator()(source_t& source, sink_t& sink, param&&... p)
	{
		return sink(source(p...));
	}
};

} //namespace detail

/**
 * \brief defines basic connection object, which is connectable.
 * \tparam source_t the source node of the connection, data flows from here to the sink.
 * \tparam sink_t is the sink node of the connection, data goes here.
 * \tparam param_void is true if the parameter of operator() of source_t is void
 * \result_void param_void is true if the result of operator() of sink_t is void
 * \payload_void param_void is true if the result of operator() of source_t is void
 * the return value of source_t needs to be convertible to the parameter of sink_t.
 */
template<
		class source_t,
		class sink_t
		>
struct connection
{
	source_t source;
	sink_t sink;


	/**
	 * \brief call operator, calls source and then sink with the result of source
	 *
	 * redundant return type specification is necessary at the moment,
	 * as it causes missing operator() in source and sink to appear in function declaration
	 * and thus be a substitution failure.
	 */
	template<class S = source_t, class T = sink_t, class... param>
	auto operator()(param&&... p)
	-> decltype(detail::invoke_helper<
				detail::void_check<S, T, param...>(0),
				S,T
			>()
			(source,sink,p...))
	{
		constexpr auto test = detail::void_check<S, T, param...>(0);
		return detail::invoke_helper<
					test,
					S,T
				>()
				(source,sink,p...);
	}

};

/// internals of flexcore, everything here can change any time.
namespace detail
{

template<class source_t, class sink_t, class Enable = void>
struct connect_impl
{
	auto operator()(const source_t& source, const sink_t& sink)
	{
		return connection<source_t, sink_t> {source, sink};
	}
};

} // namespace detail

/**
 * \brief Connect takes two connectables and returns a connection.
 *
 * \param source is the source of the data flowing through the connection.
 * \param sink is the target of the data flowing through the connection.
 * \returns connection object which has its type determined by the source_t and sink_t.
 *
 * If source_t and sink_t fulfill connectable, the result is connectable.
 * If one of source_t and sink_t fulfills receive_connectable and the other fulfills send_connectable,
 * the result is not non_connectable.
 * If either source_t or sink_t fulfill send_connectable, the result is send_connectable.
 * If either source_t or sink_t fulfill receive_connectable, the result is receive_connectable.
 */
template
	<	class source_t,
		class sink_t
	>
auto connect(const source_t& source, const sink_t& sink)
{
	return detail::connect_impl<source_t, sink_t>()(source, sink);
}

/**
 * \brief Operator >> takes two connectables and returns a connection.
 *
 * This operator is syntactic sugar for Connect.
 */
template<class source_t, class sink_t>
auto operator >>(const source_t& source, const sink_t& sink)
{
	return connect(source, sink);
}

} //namespace fc

#endif /* SRC_CORE_CONNECTION_HPP_ */
