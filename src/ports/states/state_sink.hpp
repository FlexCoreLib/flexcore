#ifndef SRC_PORTS_STATES_STATE_SINK_HPP_
#define SRC_PORTS_STATES_STATE_SINK_HPP_

// std
#include <functional>
#include <memory>

// flexcore
#include <ports/port_traits.hpp>
#include <core/connection.hpp>

namespace fc
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
		: con(std::make_shared<std::function<data_t()>>())
	{ }
	state_sink(const state_sink& other) : con(other.con) {  }

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
				"only callables can be connected");

		static_assert(std::is_same<data_t,
				typename result_of<con_t>::type>::value,
				"return value of connected needs to be data_t");
		static_assert(utils::function_traits<con_t>::arity == 0,
				"no parameter allowed for objects to be connected");
		(*con) = c;

		assert(con); //check postcondition
		assert(*con);
	}

		typedef void result_t;
private:
	std::shared_ptr<std::function<data_t()>> con;
};

// traits
template<class T> struct is_active_sink<state_sink<T> > : public std::true_type {};

}  // namespace fc

#endif /* SRC_PORTS_STATES_STATE_SINK_HPP_ */
