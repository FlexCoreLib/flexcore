#ifndef SRC_CORE_CONNECTION_HPP_
#define SRC_CORE_CONNECTION_HPP_

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
 */
template<
		class source_t,
		class sink_t,
		bool param_void,
		bool result_void,
		bool payload_void
		>
struct connection;

// metafunction which creates correct Connection type by checking
// if parameters or result types are void.
template<class source_t, class sink_t>
struct connection_trait
{

	typedef typename result_of<source_t>::type source_result;
	typedef typename result_of<sink_t>::type sink_result;

	static const bool param_is_void = utils::function_traits<source_t>::arity == 0;
	static const bool result_is_void = std::is_void<sink_result>::value;
	static const bool payload_is_void = std::is_void<source_result>::value;

	typedef connection
			<
			source_t,
			sink_t,
			param_is_void,
			result_is_void,
			payload_is_void
			> type;
};

namespace detail
{

template<class sink_t, class source_t, class Enable = void>
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
template<class source_t, class sink_t,
	class = typename std::enable_if<is_connectable<source_t>::value>::type,
	class = typename std::enable_if<is_connectable<sink_t>::value>::type>
auto connect(source_t source, sink_t sink)
{
	return detail::connect_impl<sink_t, source_t>()(source, sink);
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
 * Template specializations on param_is_void, result_is_void and payload_is_void.
 * These specializations are necessary since different parts of the function bodies
 * of operator() are ill formed if parameters, payload or results are void.
 *
 * std::enable_if on the operator() does not work in all cases, since the metafunctions
 * param_type and result_of can also be ill formed, when paramt_t and result_t are void.
 *
 */

/// Specialization in case no value is void
template<class source_t,class sink_t>
struct connection<source_t, sink_t, false, false, false>
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
struct connection<source_t, sink_t, true, false, false>
{
	source_t source;
	sink_t sink;
	auto operator()()
	{
		// execute source and execute sink with result from source.
		return sink(source());
	}
};

/// partial  specialization for no parameter and no return value
template<class source_t, class sink_t>
struct connection<source_t, sink_t, true, true, false>
{
	source_t source;
	sink_t sink;
	void operator()()
	{
		// execute source and execute sink with result from source.
		sink(source());
	}
};

/// partial specialization for no parameter and no payload
template<class source_t, class sink_t>
struct connection<source_t, sink_t, true, false, true>
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

/// partial specialization for no parameter and no return value and no payload
template<class source_t,class sink_t>
struct connection<source_t, sink_t, true, true, true>
{
	source_t source;
	sink_t sink;
	void operator()()
	{
		// execute source and execute sink separately since source has no result.
		source();
		sink();
	}
};

// Special case of connection which has no return value
template<class source_t, class sink_t>
struct connection<source_t, sink_t,false, true, false>
{
	source_t source;
	sink_t sink;
	typedef typename param_type<source_t>::type param_type;
	void operator()(const param_type& p)
	{
		// execute source with parameter and execute sink with result from source.
		sink(source(p));
	}
};

/// Special case, when there is no payload in the connnection
template<class source_t,class sink_t>
struct connection<source_t, sink_t, false, false, true>
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

/// Special case, when there is no payload and no result in the connection
template<class source_t,class sink_t>
struct connection<source_t, sink_t, false, true, true>
{
	source_t source;
	sink_t sink;
	typedef typename param_type<source_t>::type param_type;
	void operator()(const param_type& p)
	{
		source(p);
		sink();
	}
};

} //namespace fc

#include "detail/active_sink_proxy.hpp"
#include "detail/active_source_proxy.hpp"

namespace fc
{
namespace detail
{

/**
 * Specialization of connect_impl for the case of connecting a standard connectable
 * which is not a source_port to a sink_port.
 * \return a stream_proxy which contains the source and the sink.
 */
template<class sink_t, class source_t>
struct connect_impl<sink_t, source_t, typename std::enable_if<
		(!fc::is_passive_source<source_t>::value) &&
		fc::is_active_sink<sink_t>::value
		>::type >
{
	active_sink_proxy<source_t, sink_t> operator()(source_t source, sink_t sink)
	{
		return active_sink_proxy<source_t, sink_t>(source, sink);
	}
};

/**
 * Specialization for the case of connecting a source_port to a sink_port.
 * \pre source_t needs to be a stream_source.
 * \pre sink_t needs to be a stream_sink.
 * \post source is now connected to sink
 * \return nothing, the connection is complete
 */
template<class sink_t, class source_t>
struct connect_impl<sink_t, source_t, typename std::enable_if<
		fc::is_passive_source<source_t>::value &&
		fc::is_active_sink<sink_t>::value
		>::type >
{
	void operator()(source_t source, sink_t sink)
	{
		sink.connect(source);
		return;
	}
};

} // namespace detail
} // namespace fc

#endif /* SRC_CORE_CONNECTION_HPP_ */
