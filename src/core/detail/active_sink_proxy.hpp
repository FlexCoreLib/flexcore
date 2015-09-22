#ifndef SRC_PORTS_MISPLACED_STUFF_HPP_
#define SRC_PORTS_MISPLACED_STUFF_HPP_

#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>

namespace fc
{
namespace detail
{

/**
 * \brief contains connections of streams during creation.
 *
 * active_sink_proxy is used to store the intermediate objects created,
 * when a connection of streams is build up.
 * Until the stream source is connected to a proper stream_sink,
 * the connection is not complete and no value can be pulled through.
 * Nonetheless, the connection needs to be stored to allow further connections.
 * the active_sink_proxy stores these temporary objects.
 *
 *
 * \tparam source_t the source of the connection, either a connectable or a stream_source
 * \tparam sink_t the stream_sink, which serves as the active part of the connection.
 *
 * ToDo merge this with the expected code duplication, when we make the proxy for events.
 */
template<class source_t, class sink_t>
struct active_sink_proxy
{
	active_sink_proxy(source_t source, sink_t sink_) :
			stored_source(source), sink(sink_)
	{
	}

	/**
	 * \brief connects a connectable, which is not a source_port to the active_sink_proxy.
	 *
	 * connects the new_source to the current stored source
	 *
	 * \pre new_source_t needs to be connectable
	 * \post new_source is connected to the old source.
	 * \returns a active_sink_proxy, which contains the new_source as the source.
	 */
	template<class new_source_t, class = typename std::enable_if<
	        !is_passive_source<new_source_t>::value>::type>
	auto connect(new_source_t new_source)
	{
		auto connection = ::fc::connect(new_source, stored_source);
		return active_sink_proxy<decltype(connection), sink_t>(connection, sink);
	}

	/**
	 * \brief connects a source_port to the active_sink_proxy.
	 *
	 * connects the new_source to the current stored source
	 * then connects this connection to the sink which completes the connection
	 *
	 * \pre new_source_t needs to be a source_port
	 * \post new_source is now connected to sink via the connection stored in the proxy.
	 * \returns nothing, the connection is complete now
	 */
	template<class new_source_t, class enable = void>
	typename std::enable_if<is_passive_source<new_source_t>::value, void>::type
	connect(new_source_t new_source)
	{
		auto tmp = ::fc::connect(new_source, stored_source);
		sink.connect(tmp);
		return;
	}

	source_t stored_source;
	//access to connect method of sink port
	sink_t sink;
};


template<class T>
struct is_stream_proxy: std::false_type
{
};

template<class source_t, class sink_t>
struct is_stream_proxy<active_sink_proxy<source_t, sink_t>> : std::true_type
{
};

/// Specialization of connect to call member connect of active_sink_proxy.
template<class sink_t, class source_t>
struct connect_impl<sink_t, source_t,
        typename std::enable_if<is_stream_proxy<sink_t>::value>::type>
{
	auto operator()(source_t source, sink_t sink)
	{
		return sink.connect(source);
	}
};

}  // namespace detail
}  // namespace fc

#endif /* SRC_PORTS_MISPLACED_STUFF_HPP_ */
