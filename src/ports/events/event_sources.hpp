#ifndef SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_
#define SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_

#include <cassert>
#include <functional>
#include <memory>
#include <list>

#include <core/traits.hpp>
#include <core/connection.hpp>

#include <ports/connection_util.hpp>
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
 * can be connected to multiples sinks and stores these in a std::list.
 *
 * \tparam event_t type of event stored, needs to fulfill copy_constructable.
 */
template<class event_t>
struct event_source
{
	typedef std::remove_reference_t<event_t> result_t;
	typedef typename detail::handle_type<result_t>::type handler_t;

	typedef std::function<void(void)> void_fun;
	typedef std::shared_ptr<void_fun> callback_fun_ptr_strong;

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
	 * \brief Creates a callback to deregister event sinks
	 * \returns std::shared_ptr<std::function<void(void)>>, pointer to a callback method
	 * \pre sink_callback != null_ptr
	 * \pre event_source_t::event_handlers != null_ptr
	 * \pre *event_source_t::event_handlers is container that does not invalidate its iterators (e.g. std::list)
	 * \pre Method is not called in parallel i.e. sinks are connected in serial fashion
	 * \post sink_callback->empty() == false
	 */
	auto create_callback_delete_handler()
	{
		//Assumes event_wrappers::connect is not called simultaneously -> not thread safe!
		auto handleIt = std::prev(event_handlers.end());

		sink_callbacks.push_back(std::make_shared<void_fun>());
		auto callbackIt = std::prev(sink_callbacks.end());
		sink_callbacks.back()=std::make_shared<void_fun>(
				[this, handleIt, callbackIt]()
				{
					event_handlers.erase(handleIt);
					sink_callbacks.erase(callbackIt);
				}
			);

		assert(!sink_callbacks.empty());

		return sink_callbacks.back();
	}

	/**
	 * \brief default implementation of connect(source_t, conn_t)
	 * \param c the new target to be connected.
	 * \pre c is not empty function
	 * \pre c not of type event_sink
	 * \post event_handlers.empty() == false
	 */
	template<class source_t, class conn_t, class Enable = void>
	struct connect_impl {
		auto operator()(source_t source, conn_t c)
		{
			source.event_handlers.emplace_back(detail::handler_wrapper(std::forward<conn_t>(c)));

			assert(!source.event_handlers.empty());
			return port_connection<decltype(this), conn_t, result_t>();
		}
	};

	/**
	 * \brief specialization for connection with connection struct
	 * \param c the new target to be connected.
	 * \pre c is not empty function
	 * \pre c of type connection
	 * \pre c.c.(...).c has register function
	 * \post event_handlers.empty() == false
	 */
	template <class source_t, class conn_t>
	struct connect_impl
		<	source_t,
			conn_t,
			typename std::enable_if <
				has_register_function<typename get_sink_t<conn_t>::value>(0)
			>::type
		>
	{
		auto operator()(source_t source, conn_t c)
		{
			source.event_handlers.emplace_back(detail::handler_wrapper(std::forward<conn_t>(c)));

			get_sink(c).register_callback(source.create_callback_delete_handler());

			assert(!source.event_handlers.empty());
			return port_connection<decltype(this), conn_t, result_t>();
		}
	};

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
		return connect_impl<decltype(*this), conn_t>()(*this, std::forward<conn_t>(c));
	}

private:

	// stores event_handlers in shared vector, since the port is stored in a node
	// but will be copied, when it is connected. The node needs to send
	// to all connected event_handlers, when an event is fired.
	std::list<handler_t> event_handlers;
	std::list<callback_fun_ptr_strong> sink_callbacks;
};

} // namespace pure

// traits
template<class T> struct is_active_source<pure::event_source<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_ */
