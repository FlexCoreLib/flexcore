#ifndef SRC_PORTS_STATES_STATE_SINK_HPP_
#define SRC_PORTS_STATES_STATE_SINK_HPP_

// std
#include <functional>
#include <memory>

// flexcore
#include <ports/detail/port_traits.hpp>
#include <core/connection.hpp>

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
 *
 * \invariant shared_ptr<> con != null_ptr
 */
template<class data_t>
class state_sink
{
public:
	state_sink()
		: con(std::make_shared<std::function<data_t()>>())
	{ }
	state_sink(const state_sink& other) : con(other.con) { assert(con); }

	//typedef data_t result_t;

	/**
	 * \brief pulls state from connection
	 *
	 * \returns current state available at this port.
	 */
	data_t get() const { return (*con)(); }

	/**
	 * \brief Cconnects state source to sink.
	 * Sink takes ownership of the connection
	 *
	 * \pre c needs to be connectable and passive_source
	 * \post connection is not empty
	 */
	template<class con_t>
	void connect(con_t c)
	{
		static_assert(is_callable<con_t>::value,
				"only callables can be connected to a state_sink");
		static_assert(is_passive_source<con_t>::value,
				"only passive sources can be connected to a state_sink");

		(*con) = c;

		assert(con); //check postcondition
		assert(*con);
	}

		typedef void result_t;
private:
	std::shared_ptr<std::function<data_t()>> con;
};

} // namespace pure

// traits
template<class T> struct is_active_sink<pure::state_sink<T>> : public std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_STATES_STATE_SINK_HPP_ */
