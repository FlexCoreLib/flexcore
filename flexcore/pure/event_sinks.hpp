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

/// Ports and nodes which are independent of parallel regions, graph visualization etc.
namespace pure
{
/**
 * \brief Input port for events, executes given actions on incoming events.
 *
 * event_sink fulfills passive_sink.
 * \tparam event_t type of event expected, must be copy_constructable or move_constructable
 * \ingroup ports
 */
template<class event_t>
struct event_sink
{
	typedef typename detail::handle_type<event_t>::type handler_t;
	typedef void result_t;

	/**
	 * \brief Construct event_sink with action to execute in events
	 * \param action Action to execute with incoming events
	 * \pre action must be function with signature void(event_t).
	 */
	template<class action_t>
	explicit event_sink(action_t&& action) :
			event_handler(std::forward<action_t>(action))
	{
		static_assert(std::is_constructible<handler_t, action_t>(),
				"action given to event_sink needs to have signature void(event_t)."
				" Where event_t is type of token expected by event_sink.");
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

	event_sink& operator=(event_sink&& o)
	{
		assert(o.connection_breakers.empty());
		swap(o.event_handler, event_handler);
		return *this;
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

} // namespace pure
} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
