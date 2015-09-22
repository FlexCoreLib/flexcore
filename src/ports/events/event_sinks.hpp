#ifndef SRC_PORTS_EVENTS_EVENT_SINKS_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINKS_HPP_

#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>

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

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
