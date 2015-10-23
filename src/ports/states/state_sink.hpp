#ifndef SRC_PORTS_STATES_STATE_SINK_HPP_
#define SRC_PORTS_STATES_STATE_SINK_HPP_

// std
#include <functional>
#include <memory>

// flexcore
#include <core/traits.hpp>
#include <core/connection.hpp>

namespace fc
{

/**
 * Simple implementation for StreamSink.
 * Will pull data when get is called.
 * Is not a Connectable.
 */
template<class data_t>
class state_sink
{
public:
	state_sink()
		: con(std::make_shared<std::function<data_t()>>())
	{ }
	state_sink(const state_sink& other) : con(other.con) {  }

	data_t get() { return (*con)(); }

	template<class con_t>
	void connect(con_t c)
	{
		static_assert(is_callable<con_t>::value,
				"only callables can be connected");
		static_assert(std::is_same<data_t,
				typename result_of<con_t>::type>::value,
				"return value of connected needs to be data_t");
//		static_assert(std::is_void<
//				typename param_type<con_t>::type>::value,
//				"no parameter allowed for objects to be connected");
		(*con) = c;
	}

private:
	std::shared_ptr<std::function<data_t()>> con;
};

// traits
template<class T> struct is_active_sink<state_sink<T> > : public std::true_type {};

}  // namespace fc

#endif /* SRC_PORTS_STATES_STATE_SINK_HPP_ */
