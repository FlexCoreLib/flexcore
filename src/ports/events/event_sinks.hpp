#ifndef SRC_PORTS_EVENTS_EVENT_SINKS_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINKS_HPP_

#include <cassert>
#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>


namespace fc
{

template<class event_t>
struct event_in_port
{
	typedef typename detail::handle_type<event_t>::type handler_t;
	explicit event_in_port(const handler_t& handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()(event_t in_event)
	{
		std::cout << "Event_in port receiving event: " << in_event << "\n";
		assert(event_handler);
		event_handler(in_event);
	}

	event_in_port() = delete;

private:
	int lots_of_zeros[100] = {0};
	handler_t event_handler;
	int lots_of_zeros_[100] = {0};

};

template<>
struct event_in_port<void>
{
	typedef typename detail::handle_type<void>::type handler_t;
	explicit event_in_port(handler_t handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()()
	{
		assert(event_handler);
		event_handler();
	}
	event_in_port() = delete;
private:
	handler_t event_handler;
};

// traits
template<class T> struct is_port<event_in_port<T>> : public std::true_type {};
template<class T> struct is_passive_sink<event_in_port<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
