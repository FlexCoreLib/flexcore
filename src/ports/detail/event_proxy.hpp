#ifndef SRC_PORTS_DETAIL_EVENT_PROXY_HPP_
#define SRC_PORTS_DETAIL_EVENT_PROXY_HPP_

#include <functional>

#include <core/traits.hpp>
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
struct event_proxy
{
	event_proxy(source_t source_, sink_t sink) :
			source(source_),
			stored_sink(sink)
	{
	}
	template<class new_sink_t, class = typename std::enable_if<
	        !is_event_sink<new_sink_t>::value>::type>
	auto connect(new_sink_t sink)
	{
		auto connection = fc::connect(stored_sink, sink);
		return event_proxy<source_t, decltype(connection)>(source, connection);
	}

	template<class new_sink_t, class enable = void>
	typename std::enable_if<is_event_sink<new_sink_t>::value, void>::type connect(new_sink_t sink)
	{
		auto tmp = fc::connect(stored_sink, sink);
		source.connect(tmp);
		return;
	}

	source_t source;
	sink_t stored_sink;
};

namespace detail
{

template<class sink_t, class source_t>
struct connect_impl<sink_t, source_t,
        typename std::enable_if<
                fc::is_event_source<source_t>::value
                        && (!fc::is_event_sink<sink_t>::value)>::type>
{
	fc::event_proxy<source_t, sink_t> operator()(source_t source, sink_t sink)
	{
		return fc::event_proxy<source_t, sink_t>(source, sink);
	}
};

template<class sink_t, class source_t>
struct connect_impl<sink_t, source_t,
        typename std::enable_if<
                fc::is_event_source<source_t>::value
                        && fc::is_event_sink<sink_t>::value>::type>
{
	void operator()(source_t source, sink_t sink)
	{
		source.connect(sink);
		return;
	}
};

/// Specialization of connect to call member connect of stream_proxy.
template<class sink_t, class source_t>
struct connect_impl<sink_t, source_t,
        typename std::enable_if<
                is_instantiation_of<event_proxy, source_t>::value>::type>
{
	auto operator()(source_t source, sink_t sink)
	{
		return source.connect(sink);
	}
};

}  //namespace detail
}  //namespace fc

#endif /* SRC_PORTS_DETAIL_EVENT_PROXY_HPP_ */
