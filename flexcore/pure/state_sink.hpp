#ifndef SRC_PORTS_STATES_STATE_SINK_HPP_
#define SRC_PORTS_STATES_STATE_SINK_HPP_

// std
#include <functional>
#include <memory>

// flexcore
#include <flexcore/pure/detail/port_traits.hpp>
#include <flexcore/pure/detail/port_utils.hpp>
#include <flexcore/core/connection_util.hpp>
#include <flexcore/pure/detail/active_connection_proxy.hpp>

namespace fc
{
namespace pure
{

/**
 * \brief Simple implementation for input port of states
 *
 * Will pull data when get is called.
 * Is not a Connectable.
 *
 * \tparam data_t data type flowing through this port. Needs to fulfill copy_constructable
 */
template<class data_t>
class state_sink
{
public:
	state_sink()
		: con()
	{ }
	state_sink(const state_sink&) = delete;
	state_sink(state_sink&&) = default;

	//typedef data_t result_t;

	/**
	 * \brief pulls state from connection
	 *
	 * \returns current state available at this port.
	 */
	data_t get() const { return con(); }

	/**
	 * \brief Cconnects state source to sink.
	 * Sink takes ownership of the connection
	 *
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

		// optionally register a callback
		using source_t = typename get_source_t<con_t>::type;
		auto can_register_function = std::integral_constant<bool, fc::has_register_function<source_t>(0)>{};
		breaker.add_circuit_breaker(get_source(c), can_register_function);

		con = detail::handler_wrapper(std::forward<con_t>(c));
		assert(con); //check postcondition
	}

	typedef void result_t;
private:
	std::function<data_t()> con;
	detail::connection_breaker<std::function<data_t()>, detail::single_handler_policy> breaker{con};
};

} // namespace pure

// traits
template<class T> struct is_active_sink<pure::state_sink<T>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_STATES_STATE_SINK_HPP_ */
