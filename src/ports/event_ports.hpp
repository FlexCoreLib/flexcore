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

template<class event_t>
using handler_t = std::function<void(event_t)>;

template<class event_t>
struct event_out_port
{
	event_out_port()
	: event_handlers(new std::vector<handler_t<event_t>>)
	{
	}

	void fire(event_t event)
	{
		for (auto target : *event_handlers)
			target(event);
	}

	void add(std::function<void(event_t)> new_handler)
	{
		event_handlers->push_back(new_handler);
	}

private:

	typedef std::shared_ptr<std::vector<handler_t<event_t>>> handler_vector;
	handler_vector event_handlers = new handler_vector();
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
struct active_connection
{
	//ToDo, do we need an overload of connect for this?
	//yes we do, we need to create active_connections, when we connect an active connectable to a connectable.

	source_t source;
	sink_t sink;
};

template<class source_t, class sink_t>
active_connection<source_t, sink_t> connect_active(source_t source, sink_t sink)
{
	return active_connection<source_t, sink_t> {source, sink};
}


template<class source_param_t, class sink_t, class = typename std::enable_if<
        is_connectable<sink_t>::value>::type>
auto connect(event_port_handle<source_param_t> source, sink_t sink)
{
	source.connect(sink);
	return connect_active(source, sink);
}

template<class source_param_t, class sink_t, class = typename std::enable_if<
        is_connectable<sink_t>::value>::type>
auto connect(event_out_port<source_param_t> source, sink_t sink)
{
	//we can copy source here, since it stores its targets in a shared_ptr
	typedef std::function<void(handler_t<source_param_t>)> connect_function;

	connect_function tmp
			= std::bind(&event_out_port<source_param_t>::add, source, std::placeholders::_1);

	return connect(
			event_port_handle<source_param_t>(tmp),
			sink);
}

} //namespace fc

#endif /* SRC_PORTS_EVENT_PORTS_HPP_ */
