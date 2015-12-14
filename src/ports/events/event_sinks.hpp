#ifndef SRC_PORTS_EVENTS_EVENT_SINKS_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINKS_HPP_

#include <cassert>
#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>
#include "ports/detail/port_tags.hpp"
#include "ports/detail/port_traits.hpp"

namespace fc
{

/**
 * \brief minimal input port for events
 *
 * fulfills passive_sink
 * \tparam event_t type of event expected, must be copy_constructable
 */
template<class event_t>
struct event_in_port
{
	typedef typename detail::handle_type<event_t>::type handler_t;
	explicit event_in_port(const handler_t& handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()(auto&& in_event) // universal ref here?
	{
		assert(event_handler);
		event_handler(std::move(in_event));
	}

	event_in_port() = delete;

	typedef void result_t;

private:
	handler_t event_handler;

};

/// specialisation of event_in_port with void , necessary since operator() has no parameter.
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

template<class lambda_t>
struct event_in_port2
{
	explicit event_in_port2(lambda_t h) :
			lambda(h)
	{}

	void operator()(auto&& in_event) // universal ref here?
	{
		lambda(std::move(in_event));
	}

	event_in_port2() = delete;

	typedef void result_t;

private:
	lambda_t lambda;
};

template<class lambda_t>
auto make_event_in_port2(lambda_t h) { return event_in_port2<lambda_t>{h}; }
#define IN_PORT(NAME, FUNCTION) \
	auto NAME()	\
	{ return make_event_in_port2( [this](auto event){ this->FUNCTION(event); } ); } \

// traits
template<class T> struct is_port<event_in_port<T>> : public std::true_type {};
template<class T> struct is_passive_sink<event_in_port<T>> : std::true_type {};
//template<class T, class U> struct is_passive_sink<event_in_tmpl<T, U>> : std::true_type {};
template<class T> struct is_port<event_in_port2<T>> : public std::true_type {};
template<class T> struct is_passive_sink<event_in_port2<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
