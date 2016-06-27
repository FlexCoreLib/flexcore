#ifndef SRC_PORTS_EVENTS_EVENT_SINKS_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINKS_HPP_

#include <cassert>
#include <functional>
#include <vector>

#include <flexcore/core/traits.hpp>
#include <flexcore/pure/detail/port_traits.hpp>
#include <flexcore/pure/detail/active_connection_proxy.hpp>

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
	typedef void result_t;

	explicit event_sink(const handler_t& handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	template <class T>
	auto operator()(T&& in_event) -> std::enable_if_t<std::is_convertible<T&&, event_t>{}>
	{
		assert(event_handler);
		event_handler(std::forward<T>(in_event));
	}

	template <class T = event_t, typename = std::enable_if_t<std::is_void<T>{}>>
	void operator()()
	{
		assert(event_handler);
		event_handler();
	}

	event_sink(const event_sink&) = delete;
	event_sink(event_sink&& o)
	{
		assert(o.connection_breakers.empty());
		// Only move the handler so that if the assert doesn't fire (e.g.  when
		// NDEBUG is defined) the moved-from-object can still disconnect
		// itself.
		swap(o.event_handler, event_handler);
	}

	~event_sink()
	{
		auto self = std::hash<decltype(this)>{}(this);
		for (auto& breaker_ptr : connection_breakers)
		{
			auto breaker = breaker_ptr.lock();
			if (breaker)
				(*breaker)(self);
		}
	}

	void register_callback(std::shared_ptr<std::function<void(size_t)>>& visit_fun)
	{
		assert(visit_fun);
		assert(*visit_fun);
		connection_breakers.emplace_back(visit_fun);
	}

private:
	handler_t event_handler;
	std::vector<std::weak_ptr<std::function<void(size_t)>>> connection_breakers;
};

/**
 * \brief Templated event sink port
 *
 * Calls a (generic) lambda when an event is received. This allows to defer
 * the actual type of the token until the port is called and also allows it
 * to be called for diffent types.
 *
 * See tests/pure/test_events.cpp for example
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
} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
