#ifndef SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_
#define SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_

#include <cassert>
#include <functional>
#include <memory>
#include <vector>

#include <core/traits.hpp>
#include <core/connection.hpp>

#include <ports/detail/port_traits.hpp>
#include <ports/detail/port_utils.hpp>
#include <core/detail/active_connection_proxy.hpp>

#include <iostream>

namespace fc
{
namespace pure
{

/**
 * \brief minimal output port for events.
 *
 * fulfills active_source
 * can be connected to multiples sinks and stores these in an std::vector.
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
		event_handlers.emplace_back(detail::handler_wrapper(std::forward<conn_t>(c)));

		assert(!event_handlers.empty());
		return port_connection<decltype(*this), handler_t, result_t>();
	}

private:
	std::vector<handler_t> event_handlers;
};

} // namespace pure

// traits
template<class T> struct is_active_source<pure::event_source<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_ */
