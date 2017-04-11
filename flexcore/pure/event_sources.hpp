#ifndef SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_
#define SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_

#include <cassert>
#include <functional>
#include <memory>
#include <vector>

#include <flexcore/core/traits.hpp>
#include <flexcore/core/connection_util.hpp>
#include <flexcore/pure/detail/port_traits.hpp>
#include <flexcore/pure/detail/port_utils.hpp>
#include <flexcore/pure/port_connection.hpp>
#include <flexcore/pure/detail/active_connection_proxy.hpp>

namespace fc
{
namespace pure
{

/**
 * \brief Output port for events.
 *
 * event_source fulfills active_source.
 * can be connected to multiples sinks and stores these in a std::list.
 *
 * \remark This class is not thread safe with respect to connections
 * i.e. all connections to sinks must be made serially
 *
 * \tparam event_t type of event stored,
 * needs to fulfill copy_constructable or move_constructable.
 * \ingroup ports
 */
template<class event_t>
struct event_source
{
	typedef std::remove_reference_t<event_t> result_t;
	typedef event_t token_t;
	typedef typename detail::handle_type<result_t>::type handler_t;

	event_source() = default;

	/**
	 * \brief Sends parameter as event to all connected conntables and event_sinks.
	 * \param event token to be sent through this port.
	 */
	template<class... T>
	void fire(T&&... event)
	{
		static_assert(sizeof...(T) == 0 || sizeof...(T) == 1,
				"we only allow single events, or void events atm");

		static_assert(std::is_void<event_t>{} ||
		              std::is_constructible<event_t, T...>{},
		              "tried to call fire with a type, not implicitly convertible to type of port."
		              "If conversion is required, do the cast before calling fire.");

		for (auto& target : base.storage.handlers)
		{
			assert(target);
			target(static_cast<event_t>(event)...);
		}
	}

	/// Gives the number of connections from this port.
	size_t nr_connected_handlers() const
	{
		return base.storage.handlers.size();
	}

	/**
	 * \brief connects new connectable target to port.
	 *
	 * Optionally adds a callback to deregister the connection
	 * if supported by the sink at the end of the chain of connectables
	 * \param c the new target to be connected.
	 * \pre c is not empty function
	 * \post base.storage.event_handlers.empty() == false
	 */
	template <class conn_t>
	auto connect(conn_t&& c) &
	{
		static_assert(detail::has_result_of_type<conn_t, event_t>(),
			"The type returned by this source is not compatible with the connection you "
			"are trying to establish.");

		base.add_handler(detail::handler_wrapper(std::forward<conn_t>(c)), get_sink(c));

		assert(!base.storage.handlers.empty());
		return port_connection<decltype(this), conn_t, result_t>();
	}

	///Illegal overload for rvalue port to give better error message.
	template<class con_t>
	void connect(con_t&&) &&
	{
		static_assert(detail::always_false<con_t>(),
				"Illegally tried to connect a temporary event_source object.");
	}

private:
	// Stores event_handlers in a vector, the node needs to send
	// to all connected event_handlers when an event is fired.
	detail::active_port_base<handler_t, detail::multiple_handler_policy> base;
};

} // namespace pure

/// event_source is the essential active_source
template<class T> struct is_active_source<pure::event_source<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_ */
