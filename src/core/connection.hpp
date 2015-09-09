#ifndef SRC_CORE_CONNECTION_HPP_
#define SRC_CORE_CONNECTION_HPP_

#include <type_traits>

#include "core/function_traits.hpp"
#include "core/traits.hpp"

/// Holds ownership of source and sink, used by connections as mixin to avoid code duplication.
template<class source_t, class sink_t>
struct SinkSourceOwner
{
	source_t source;
	sink_t sink;
};

/**
 * \brief defines basic connection object, which is connectable.
 *
 */
template<
		class source_t,
		class sink_t,
		bool param_void,
		bool result_void,
		bool payload_void
		>
struct Connection;

template<
		class source_t,
		class sink_t
		>
struct Connection<source_t, sink_t, false, false, false>
		: public SinkSourceOwner<source_t, sink_t>
{
	typedef typename ParamType<source_t>::type param_type;
	auto operator()(const param_type&& p)
	{
		// execute source with parameter and execute sink with result from source.
		return this->sink(this->source(p));
	}
};

// partial  specialization for no parameter
template<
		class source_t,
		class sink_t
		>
struct Connection<source_t, sink_t, true, false, false>
		: public SinkSourceOwner<source_t, sink_t>
{
	auto operator()()
	{
		// execute source and execute sink with result from source.
		return this->sink(this->source());
	}
};

//This is the special case, when there is no payload in the connnection
template<
		class source_t,
		class sink_t
		>
struct Connection<source_t, sink_t, false, false, true>
		: public SinkSourceOwner<source_t, sink_t>
{
	typedef typename ParamType<source_t>::type param_type;
	auto operator()(const param_type&& p)
	{
		// execute source and execute sink separately.
		this->source(p);
		return this->sink();
	}
};


/// partial specialization for no parameter and no payload
template<class source_t, class sink_t>
struct Connection<source_t, sink_t, true, false, true>
		: public SinkSourceOwner<source_t, sink_t>
{
	void operator()()
	{
		// execute source and execute sink separately since source has no result.
		this->source();
		return this->sink();
	}
};

// Special case of connection which has no return value
template<class source_t, class sink_t>
struct Connection<source_t, sink_t,false, true, false>
		: public SinkSourceOwner<source_t, sink_t>
{
	typedef typename ParamType<source_t>::type param_type;
	void operator()(const param_type&& p)
	{
		// execute source with parameter and execute sink with result from source.
		this->sink(this->source(p));
	}
};

// partial  specialization for no parameter and no return value
template<class source_t, class sink_t>
struct Connection<source_t, sink_t, true, true, false>
		: public SinkSourceOwner<source_t, sink_t>
{
	void operator()()
	{
		// execute source and execute sink with result from source.
		this->sink(this->source());
	}
};

/// partial specialization for no parameter and no return value and no payload
template<
		class source_t,
		class sink_t
		>
struct Connection<source_t, sink_t, true, true, true>
		: public SinkSourceOwner<source_t, sink_t>
{
	void operator()()
	{
		// execute source and execute sink separately since source has no result.
		this->source();
		this->sink();
	}
};

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

	typedef Connection
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
template<class source_t, class sink_t>
typename connection_trait<source_t, sink_t>::type connect_impl
		(
		const source_t& source,
		const sink_t& sink
		)
{
	return typename connection_trait<source_t, sink_t>::type { source, sink };
}
} // namespace detail

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
auto connect(const source_t& source, const sink_t& sink)
{
	return detail::connect_impl(source, sink);
}

/**
 * \brief Operator >> takes two connectables and returns a connection.
 *
 * This operator is syntactic sugar for Connect.
 */
template<class sink_t, class source_t>
auto operator >>(const source_t& source, const sink_t& sink)
{
	return Connect(source, sink);
}

#endif /* SRC_CORE_CONNECTION_HPP_ */
