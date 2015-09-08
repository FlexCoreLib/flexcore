/*
 * connection.hpp
 *
 *  Created on: Sep 8, 2015
 *      Author: ckielwein
 */

#ifndef SRC_CORE_CONNECTION_HPP_
#define SRC_CORE_CONNECTION_HPP_

#include <type_traits>

#include "core/function_traits.hpp"
#include "core/traits.hpp"

template<class source_t, class sink_t, class sink_result,
		class param_type>
struct Connection {
	source_t source;
	sink_t sink;
	sink_result operator()(const param_type&& p) {
		//execute source with parameter and execute sink with result from source.
		return sink(source(p));
	}
};

// partial  specialization for no parameter
template<class source_t, class sink_t, class sink_result>
struct Connection<source_t, sink_t, sink_result, void> {
	source_t source;
	sink_t sink;
	sink_result operator()() {
		//execute source and execute sink with result from source.
		return sink(source());
	}
};


// Special case of connection which has no return value
template<class source_t, class sink_t, class param_type>
struct VoidConnection {
	source_t source;
	sink_t sink;
	void operator()(const param_type&& p) {
		//execute source with parameter and execute sink with result from source.
		sink(source(p));
	}
};

// partial  specialization for no parameter and no return value
template<class source_t, class sink_t>
struct VoidConnection<source_t, sink_t, void> {
	source_t source;
	sink_t sink;
	void operator()() {
		//execute source and execute sink with result from source.
		sink(source());
	}
};


template<bool no_arg, class T>
struct ParamType {
};

template<class T>
struct ParamType<true, T> {
	typedef void type;
};

template<class T>
struct ParamType<false, T> {
	typedef typename utils::function_traits<T>
	::template arg<0>::type type;
};

// metafunction which creates correct Connection type by checking if parameters or result types are void.
template<class source_t, class sink_t>
struct connection_trait {
	static const bool source_no_arg = utils::function_traits<source_t>::arity == 0;

	typedef typename ParamType<source_no_arg, source_t>::type source_param;
	typedef typename result_of<sink_t>::type sink_result;

	static const bool result_is_void = std::is_void<sink_result>::value;
	typedef typename std::conditional<result_is_void,
			VoidConnection<source_t, sink_t, source_param>,
			Connection<source_t, sink_t, sink_result, source_param>>::type type;
};


namespace detail
{
template<class source_t, class sink_t>
typename connection_trait<source_t, sink_t>::type Connect_impl(
		const source_t& source, const sink_t& sink) {
	return typename connection_trait<source_t, sink_t>::type { source,
			sink };
}
}

/**
 * \brief Connect takes two connectables and returns a connection.
 *
 * \param source is the source of the data flowing through the connection.
 * \param sink is the target of the data flowing through the connection.
 * \returns connection object which has its type determined by the source_t and sink_t.
 *
 * If source_t and sink_t fulfill connectable, the result is connectable.
 * If one of source_t and sink_t fulsulls receive_connectable and the other fulfills send_connectable,
 * the result is not non_connectable.
 * If either source_t or sink_t fulfill send_connectable, the result is send_connectable.
 * If either source_t or sink_t fulfill receive_connectable, the result is receive_connectable.
 */
template<class sink_t, class source_t>
auto connect(const source_t& source, const sink_t& sink) {
	return detail::connect_impl(source, sink);
}

/**
 * \brief Operator >> takes two connectables and returns a connection.
 *
 * This operator is syntactic sugar for Connect.
 */
template<class sink_t, class source_t>
auto operator >>(const source_t& source, const sink_t& sink) {
	return Connect(source, sink);
}

#endif /* SRC_CORE_CONNECTION_HPP_ */
