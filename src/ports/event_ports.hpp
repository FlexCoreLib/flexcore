#ifndef SRC_PORTS_EVENT_PORTS_HPP_
#define SRC_PORTS_EVENT_PORTS_HPP_

#include <functional>

#include "core/traits.hpp"
#include "core/connection.hpp"

namespace fc
{

template<class event_t>
struct event_in_port
{
	event_in_port(std::function<void(event_t)> handler) :
			event_handler(handler)
	{
	}

	void operator()(event_t in_event)
	{
		event_handler(in_event);
	}

private:
	std::function<void(event_t)> event_handler;
};

template<class T>
struct is_event_sink<event_in_port<T>> : public std::true_type
{
};

template<class event_t>
using handler_t = std::function<void(event_t)>;

template<class event_t>
struct event_out_port
{
	event_out_port() :
			event_handlers(new std::vector<handler_t<event_t>>)
	{
	}

	void fire(event_t event)
	{
		for (auto target : *event_handlers)
			target(event);
	}

	void connect(std::function<void(event_t)> new_handler)
	{
		event_handlers->push_back(new_handler);
	}

private:

	typedef std::shared_ptr<std::vector<handler_t<event_t>>>handler_vector;
	handler_vector event_handlers = new handler_vector();
};

template<class T>
struct is_event_source<event_out_port<T>> : public std::true_type
{
};

/**
 * \brief wraps access to an event_in_port in connection objects.
 *
 * Purpose of event_port_handle is to allow the necessary access to the port
 * without transferring ownership of the port to the connection.
 * An object of this type can be copied into and owned in connections of event_in_ports.
 * It contains a callback to the connect function of the port.
 *
 * event_port_handle is active_connectable, but not callable.
 */
template<class event_t>
struct event_port_handle
{
	event_port_handle(std::function<void(handler_t<event_t>)> port_access) :
			connect(port_access)
	{
	}

	std::function<void(handler_t<event_t>)> connect;
};

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

#endif /* SRC_PORTS_EVENT_PORTS_HPP_ */
