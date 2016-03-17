#ifndef SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_
#define SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_

#include <cassert>
#include <functional>
#include <memory>
#include <vector>

#include <core/traits.hpp>
#include <core/ports.hpp>

#include <ports/connection_util.hpp>
#include <ports/detail/port_traits.hpp>
#include <ports/detail/port_utils.hpp>

#include <iostream>

namespace fc
{
namespace pure
{

/**
 * \brief minimal output port for events.
 *
 * fulfills active_source
 * can be connected to multiples sinks and stores these in a std::list.
 *
 * \remark This class is not thread safe with respect to connections
 * i.e. all connections to sinks must be made serially
 *
 * \tparam event_t type of event stored, needs to fulfill copy_constructable.
 */
template<class event_t>
struct event_source
{
	typedef std::remove_reference_t<event_t> result_t;
	typedef typename detail::handle_type<result_t>::type handler_t;

	event_source() = default;
	event_source(const event_source&) = delete;
	event_source(event_source&&) = default;

	template<class... T>
	void fire(T&&... event)
	{
		static_assert(sizeof...(T) == 0 || sizeof...(T) == 1,
				"we only allow single events, or void events atm");

		static_assert(std::is_void<event_t>{} ||
		              std::is_constructible<event_t, T...>{},
		              "tried to call fire with a type, not implicitly convertible to type of port."
		              "If conversion is required, do the cast before calling fire.");

		for (auto& target : event_handlers)
		{
			assert(target);
			target(static_cast<event_t>(event)...);
		}
	}

	size_t nr_connected_handlers() const
	{
		return event_handlers.size();
	}

	/**
	 * \brief connects new connectable target to port.
	 * Optionally adds a callback to deregister the connection
	 * if supported by the sink at the end of the chain of connectables
	 * \param new_handler the new target to be connected.
	 * \pre new_handler is not empty function
	 * \post event_handlers.empty() == false
	 */
	template <class conn_t>
	auto connect(conn_t&& c) &
	{
		static_assert(detail::has_result_of_type<conn_t, event_t>(),
			"The type returned by this source is not compatible with the connection you "
			"are trying to establish.");

		// Register a connection breaker callback if sink_t supports it
		using sink_t = typename get_sink_t<conn_t>::type;
		auto can_register_function = std::integral_constant<bool, fc::has_register_function<sink_t>(0)>{};
		breaker.add_circuit_breaker(get_sink(c), can_register_function);

		event_handlers.emplace_back(detail::handler_wrapper(std::forward<conn_t>(c)));
		assert(!event_handlers.empty());
		return port_connection<decltype(this), conn_t, result_t>();
	}

private:

	// Stores event_handlers in a vector, the node needs to send
	// to all connected event_handlers when an event is fired.
	std::vector<handler_t> event_handlers;
	detail::connection_breaker<handler_t, detail::multiple_handler_policy> breaker{event_handlers};
};

} // namespace pure

// traits
template<class T> struct is_active_source<pure::event_source<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_ */
