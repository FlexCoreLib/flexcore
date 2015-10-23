#ifndef SRC_CORE_DETAIL_CONNECTION_HPP_
#define SRC_CORE_DETAIL_CONNECTION_HPP_

#include <type_traits>

#include "core/function_traits.hpp"
#include "core/traits.hpp"

namespace fc
{

/**
 * \brief defines basic connection object, which is connectable.
 * \tparam source_t the source node of the connection, data flows from here to the sink.
 * \tparam sink_t is the sink node of the connection, data goes here.
 * \tparam param_void is true if the parameter of operator() of source_t is void
 * \result_void param_void is true if the result of operator() of sink_t is void
 * \payload_void param_void is true if the result of operator() of source_t is void
 * the return value of source_t needs to be convertible to the parameter of sink_t.
 *
 * TODO: warum brauchen wir die ..._void template args?
 */
template<
		class source_t,
		class sink_t,
		bool param_void,
		bool payload_void
		>
struct connection;

/**
 * metafunction which creates correct Connection type by checking
 * if parameters or result types are void.
 */
template<class source_t, class sink_t>
struct connection_trait
{
	typedef typename result_of<source_t>::type source_result;
	typedef typename result_of<sink_t>::type sink_result;

	static const bool param_is_void = utils::function_traits<source_t>::arity == 0;
	static const bool result_is_void = std::is_void<sink_result>::value;
	static const bool payload_is_void = std::is_void<source_result>::value;

	typedef connection
		<	source_t,
			sink_t,
			param_is_void,
			payload_is_void
		> type;
};

namespace detail
{

template<class source_t, class sink_t, class Enable = void>
struct connect_impl
{
	typename connection_trait<source_t, sink_t>::type
	operator()(const source_t& source, const sink_t& sink)
	{
		return typename connection_trait<source_t, sink_t>::type {source, sink};
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
auto connect(source_t source, sink_t sink)
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

/***************** Implementation *********************************************
 *
 * Template specializations on param_is_void and payload_is_void.
 * These specializations are necessary since different parts of the function bodies
 * of operator() are ill formed if parameter or payload are void.
 * since return void is legal in generic code, we do not need to specialize on return type.
 *
 * std::enable_if on the operator() does not work in all cases, since the metafunctions
 * param_type can also be ill formed, when paramt_t is void.
 *
 */

/// Specialization in case no value is void
template<class source_t,class sink_t>
struct connection<source_t, sink_t, false, false>
{
	source_t source;
	sink_t sink;
	typedef typename param_type<source_t>::type param_type;
	auto operator()(const param_type& p)
	{
		// execute source with parameter and execute sink with result from source.
		return sink(source(p));
	}
};

/// Partial specialization no parameter
template<class source_t,class sink_t>
struct connection<source_t, sink_t, true, false>
{
	source_t source;
	sink_t sink;
	auto operator()()
	{
		// execute source and execute sink with result from source.
		return sink(source());
	}
};

/// partial specialization for no parameter and no payload
template<class source_t, class sink_t>
struct connection<source_t, sink_t, true, true>
{
	source_t source;
	sink_t sink;
	auto operator()()
	{
		// execute source and execute sink separately since source has no result.
		source();
		return sink();
	}
};

/// Special case, when there is no payload in the connnection
template<class source_t,class sink_t>
struct connection<source_t, sink_t, false, true>
{
	source_t source;
	sink_t sink;
	typedef typename param_type<source_t>::type param_type;
	auto operator()(const param_type& p)
	{
		// execute source and execute sink separately.
		source(p);
		return sink();
	}
};

} //namespace fc

#endif /* SRC_CORE_CONNECTION_HPP_ */
