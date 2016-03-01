#ifndef SRC_PORTS_EVENTS_EVENT_SINKS_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINKS_HPP_

#include <cassert>
#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>
#include <ports/detail/port_traits.hpp>

namespace fc
{
namespace pure
{

/**
 * \brief minimal input port for events
 *
 * fulfills passive_sink
 * \tparam event_t type of event expected, must be copy_constructable
 */
template<class event_t>
struct event_sink
{
	typedef typename detail::handle_type<event_t>::type handler_t;
	explicit event_sink(const handler_t& handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	template <class T>
	void operator()(T&& in_event)
	{
		assert(event_handler);
		event_handler(std::forward<T>(in_event));
	}

	event_sink() = delete;
	event_sink(const event_sink&) = delete;
	event_sink(event_sink&&) = default;

	typedef void result_t;

private:
	handler_t event_handler;

};

/// specialisation of event_sink with void , necessary since operator() has no parameter.
template<>
struct event_sink<void>
{
	typedef typename detail::handle_type<void>::type handler_t;
	explicit event_sink(handler_t handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()()
	{
		assert(event_handler);
		event_handler();
	}

	event_sink() = delete;
	event_sink(const event_sink&) = delete;
	event_sink(event_sink&&) = default;
private:
	handler_t event_handler;
};

/**
 * \brief Templated event sink port
 *
 * Calls a (generic) lambda when an event is received. This allows to defer
 * the actual type of the token until the port is called and also allows it
 * to be called for diffent types.
 *
 * See tests/ports/events/test_events.cpp for example
 *
 * \tparam lambda_t Lambda to call when event arrived.
 */
template<class lambda_t>
struct event_sink_tmpl
{
public:
	explicit event_sink_tmpl(lambda_t h)
		: lambda(h)
	{}

	template <class Event_t>
	void operator()(Event_t&& in_event)
	{
		lambda(std::forward<Event_t>(in_event));
	}

	event_sink_tmpl() = delete;

	typedef void result_t;

	lambda_t lambda;
};

/*
 * Helper needed for type inference
 */
template<class lambda_t>
auto make_event_sink_tmpl(lambda_t h) { return event_sink_tmpl<lambda_t>{h}; }

} // namespace pure

// traits
template<class T> struct is_passive_sink<pure::event_sink<T>> : std::true_type {};
template<class T> struct is_passive_sink<pure::event_sink_tmpl<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
