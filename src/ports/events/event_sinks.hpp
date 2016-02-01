#ifndef SRC_PORTS_EVENTS_EVENT_SINKS_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINKS_HPP_

#include <cassert>
#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>
#include "ports/detail/port_traits.hpp"

#include <iostream>

namespace fc
{

/// Mixin for executing a callback to the source on deletion of the sink
struct event_in_port_callback_mixin
{
	typedef std::weak_ptr<std::function<void(void)>> callback_fun_ptr_weak;
	explicit event_in_port_callback_mixin() :
		deregisterer(std::make_shared<scoped_deregister>())
	{
	}

public:

	/**
	 * \brief Registers the source to the sink
	 * \pre fun != null_ptr
	 * \post destruct_callback->expired() == false
	 */
	void register_callback(const std::shared_ptr<std::function<void(void)>>& fun)
	{
		assert(fun);
		deregisterer->destruct_callback = fun;
		assert(!deregisterer->destruct_callback.expired());
	}

	long int numRefs()
	{
		return deregisterer.use_count();
	}

private:
	/**
	 * \brief scoped wrapper around deregister call
	 *
	 * Makes sure deregister is only called, when the last copy of the port is destroyed
	 */
	struct scoped_deregister
	{
		~scoped_deregister()
		{
			deregister();
		}

		/**
		 * \brief Deregisters from the source
		 * \pre destruct_callback != null_ptr
		 */
		void deregister()
		{
			if (auto sharedCallbackPtr = destruct_callback.lock())
				(*sharedCallbackPtr)();
		}

		callback_fun_ptr_weak destruct_callback = callback_fun_ptr_weak();
	};

	std::shared_ptr<scoped_deregister> deregisterer;
};


/**
 * \brief minimal input port for events
 *
 * fulfills passive_sink
 * \tparam event_t type of event expected, must be copy_constructable
 */
template<class event_t>
struct event_in_port : public event_in_port_callback_mixin
{
	typedef typename detail::handle_type<event_t>::type handler_t;

	explicit event_in_port(const handler_t& handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()(auto&& in_event)
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
struct event_in_port<void> : public event_in_port_callback_mixin
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
struct event_in_port_tmpl
{
public:
	explicit event_in_port_tmpl(lambda_t h)
		: lambda(h)
	{}

	void operator()(auto&& in_event) // universal ref here?
	{
		lambda(std::move(in_event));
	}

	event_in_port_tmpl() = delete;

	typedef void result_t;

	lambda_t lambda;
};

/*
 * Helper needed for type inference
 */
template<class lambda_t>
auto make_event_in_port_tmpl(lambda_t h) { return event_in_port_tmpl<lambda_t>{h}; }

// traits
template<class T> struct is_passive_sink<event_in_port<T>> : std::true_type {};
template<class T> struct is_passive_sink<event_in_port_tmpl<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
