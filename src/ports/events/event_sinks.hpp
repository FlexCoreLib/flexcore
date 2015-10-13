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
	typedef typename detail::handle_type<event_t>::type handler_t;
	explicit event_in_port(handler_t handler) :
			event_handler(handler)
	{
	}

	template<class... T>
	void operator()(T... in_event)
	{
		event_handler(in_event...);
	}

private:
	handler_t event_handler;
};

// traits
template<class T> struct is_passive_sink<event_in_port<T>> : public std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
