#ifndef SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_
#define SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_

#include <functional>

#include <ports/port_traits.hpp>
#include <core/connection.hpp>

namespace fc
{

namespace detail
{


/**
 * \brief connection of an active_connectable and a connectable
 * fulfills active_connectable
 */
template<class source_t, class sink_t>
struct active_source_proxy
{
	typedef typename result_of<source_t>::type payload_t;
	typedef typename result_of<sink_t>::type result_type;

	active_source_proxy(source_t source_, sink_t sink) :
			source(source_),
			stored_sink(sink)
	{}

	template<	class new_sink_t,
				class enable = void>
	auto
	connect(new_sink_t sink, typename std::enable_if< is_passive_sink<new_sink_t>::value, void >::type* = 0)
	{
		auto tmp = ::fc::connect(stored_sink, sink);

		return source.connect(tmp);
	}

	template<	class new_sink_t,
				class = typename std::enable_if< not is_passive_sink<new_sink_t>::value >::type
			>
	auto connect(new_sink_t sink)
	{
		auto connection = ::fc::connect(stored_sink, sink);
		return active_source_proxy<source_t, decltype(connection)>(source, connection);
	}

	source_t source;
	sink_t stored_sink;
};

template<class source_t, class sink_t>
struct connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if
			<	is_active_source<source_t>::value
			and	(!::fc::is_passive_sink<sink_t>::value)
			>::type
	>
{
	static_assert(is_callable<sink_t>::value, "trying to connect an active source to a sink, which is not callable."
			"most likely the operator() is missing.");
	active_source_proxy<source_t, sink_t> operator()(source_t source, sink_t sink)
	{
		return active_source_proxy<source_t, sink_t>(source, sink);
	}
};


template<class source_t, class sink_t>
struct connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if
			<	::fc::is_active_source<source_t>::value
			and ::fc::is_passive_sink<sink_t>::value
			>::type
	>
{
	auto operator()(source_t source, sink_t sink)
	{
		source.connect(sink);
		return port_connection<source_t, sink_t>();
	}
};

/// Specialization of connect to call member connect of stream_proxy.
template<class source_t, class sink_t>
struct connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if
			<	is_instantiation_of< active_source_proxy,
									 source_t >::value
			>::type
	>
{
	auto operator()(source_t source, sink_t sink)
	{
		return source.connect(sink);
	}
};

}  //namespace detail
}  //namespace fc

#endif /* SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_ */
