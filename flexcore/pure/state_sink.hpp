#ifndef SRC_PORTS_STATES_STATE_SINK_HPP_
#define SRC_PORTS_STATES_STATE_SINK_HPP_

#include <flexcore/core/connection_util.hpp>
#include <flexcore/core/exceptions.hpp>
#include <flexcore/pure/detail/port_traits.hpp>
#include <flexcore/pure/detail/port_utils.hpp>
#include <flexcore/pure/detail/active_connection_proxy.hpp>

#include <functional>
#include <memory>

namespace fc
{
namespace pure
{

/**
 * \brief Simple implementation for input port of states
 *
 * Will pull data when get is called.
 * state_sink Fulfills active_sink.
 *
 * \tparam data_t data type flowing through this port.
 * Needs to fulfill copy_constructable
 * \ingroup ports
 */
template<class data_t>
class state_sink
{
public:
	state_sink() = default;

	/**
	 * \brief pulls state from connection
	 *
	 * \returns current state available at this port.
	 * \throws no_connected exception if called with an unconnected state sink.
	 */
	data_t get() const
	{
		if (!base.storage.handlers) //handlers is std::function with operator bool
			throw not_connected(
					"tried to pull data through a state_sink"
					" which is not connected");
		return base.storage.handlers();
	}

	/**
	 * \brief Connects state source to state_sink.
	 *
	 * This state_sink takes ownership of the connection.
	 * \param c connectable which is to be connected to this port.
	 * \pre c needs to be connectable and passive_source
	 * \post connection is not empty
	 */
	template<class con_t>
	void connect(con_t&& c) &
	{
		static_assert(is_callable<std::remove_reference_t<con_t>>{},
				"only callables can be connected to a state_sink");
		static_assert(is_passive_source<con_t>{},
				"only passive sources can be connected to a state_sink");

		static_assert(std::is_convertible<decltype(std::declval<con_t>()()), data_t>{},
		              "The type returned by this connection is incompatible with this sink.");

		base.add_handler(detail::handler_wrapper(std::forward<con_t>(c)), get_source(c));
	}

	///Illegal overload for rvalue port to give better error message.
	template<class con_t>
	void connect(con_t&&) &&
	{
		static_assert(detail::always_false<con_t>(),
				"Illegally tried to connect a temporary state_sink object.");
	}

	typedef void result_t;
	typedef data_t token_t;
private:
	detail::active_port_base<std::function<data_t()>, detail::single_handler_policy> base;
};

} // namespace pure

/// state_sink is the essential active_sink
template<class T> struct is_active_sink<pure::state_sink<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_STATES_STATE_SINK_HPP_ */
